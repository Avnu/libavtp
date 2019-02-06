/*
 * Copyright (c) 2019, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of Intel Corporation nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* IEC 61883/IIDC Listener example.
 *
 * This example implements a very simple IEC 61883/IIDC listener application
 * which receives AVTP packets from the network, retrieves the MPEG-TS packets,
 * and writes them to stdout once the presentation time is reached.
 *
 * For simplicity, the example supports MPEG-TS streams, and expects that each
 * AVTP packet contains only one source packet.
 *
 * TSN stream parameters such as destination mac address are passed via
 * command-line arguments. Run 'ieciidc-listener --help' for more information.
 *
 * This example relies on the system clock to schedule MPEG-TS packets for
 * playback. So make sure the system clock is synchronized with the PTP
 * Hardware Clock (PHC) from your NIC and that the PHC is synchronized with
 * the PTP time from the network. For further information on how to synchronize
 * those clocks see ptp4l(8) and phc2sys(8) man pages.
 *
 * The easiest way to use this example is by combining it with a GStreamer
 * pipeline. We provide an MPEG-TS stream that is sent to stdout, from where
 * GStreameer reads the stream. So, to send the MPEG-TS stream received from
 * the TSN network to GStreamer, you can do something like:
 *
 * $ ieciidc-listener <args> | gst-launch-1.0 -e -q filesrc location=/dev/stdin
 *  ! tsdemux ! decodebin ! videoconvert ! autovideosink
 */

#include <assert.h>
#include <argp.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <inttypes.h>

#include "avtp.h"
#include "avtp_ieciidc.h"
#include "examples/common.h"

#define STREAM_ID		0xAABBCCDDEEFF0001
#define MPEG_TS_PACKET_LEN	188

#define DATA_LEN		(MPEG_TS_PACKET_LEN + sizeof(uint32_t)) /* MPEG-TS size + SPH */
#define CIP_HEADER_LEN		(sizeof(uint32_t) * 2)
#define STREAM_DATA_LEN		DATA_LEN + CIP_HEADER_LEN
#define PDU_SIZE		(sizeof(struct avtp_stream_pdu) + CIP_HEADER_LEN + DATA_LEN)

struct packet_entry {
	STAILQ_ENTRY(packet_entry) entries;

	struct timespec tspec;
	uint8_t mpegts_packet[MPEG_TS_PACKET_LEN];
};

static STAILQ_HEAD(packet_queue, packet_entry) packets;
static char ifname[IFNAMSIZ];
static uint8_t macaddr[ETH_ALEN];
static uint8_t expected_seq;
static uint8_t expected_dbc;

static struct argp_option options[] = {
	{"dst-addr", 'd', "MACADDR", 0, "Stream Destination MAC address" },
	{"ifname", 'i', "IFNAME", 0, "Network Interface" },
	{ 0 }
};

static error_t parser(int key, char *arg, struct argp_state *state)
{
	int res;

	switch (key) {
	case 'd':
		res = sscanf(arg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
					&macaddr[0], &macaddr[1], &macaddr[2],
					&macaddr[3], &macaddr[4], &macaddr[5]);
		if (res != 6) {
			fprintf(stderr, "Invalid address\n");
			exit(EXIT_FAILURE);
		}

		break;
	case 'i':
		strncpy(ifname, arg, sizeof(ifname) - 1);
		break;
	}

	return 0;
}

static struct argp argp = { options, parser };

/* Schedule 'MPEG-TS packet' to be presented at time specified by 'tspec'. */
static int schedule_packet(int fd, struct timespec *tspec, uint8_t *mpeg_tsp)
{
	struct packet_entry *entry;

	entry = malloc(sizeof(*entry));
	if (!entry) {
		fprintf(stderr, "Failed to allocate memory\n");
		return -1;
	}

	entry->tspec.tv_sec = tspec->tv_sec;
	entry->tspec.tv_nsec = tspec->tv_nsec;
	memcpy(entry->mpegts_packet, mpeg_tsp, MPEG_TS_PACKET_LEN);

	STAILQ_INSERT_TAIL(&packets, entry, entries);

	/* If this was the first entry inserted onto the queue, we need to arm
	 * the timer.
	 */
	if (STAILQ_FIRST(&packets) == entry) {
		int res;

		res = arm_timer(fd, tspec);
		if (res < 0) {
			STAILQ_REMOVE(&packets, entry, packet_entry, entries);
			free(entry);
			return -1;
		}
	}

	return 0;
}

static bool is_valid_packet(struct avtp_stream_pdu *pdu)
{
	struct avtp_common_pdu *common = (struct avtp_common_pdu *) pdu;
	uint64_t val64;
	uint32_t val32;
	int res;

	res = avtp_pdu_get(common, AVTP_FIELD_SUBTYPE, &val32);
	assert(res == 0);
	if (val32 != AVTP_SUBTYPE_61883_IIDC) {
		fprintf(stderr, "Subtype mismatch: expected %u, got %u\n",
						AVTP_SUBTYPE_61883_IIDC, val32);
		return false;
	}

	res = avtp_pdu_get(common, AVTP_FIELD_VERSION, &val32);
	assert(res == 0);
	if (val32 != 0) {
		fprintf(stderr, "Version mismatch: expected %u, got %u\n",
								0, val32);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_TV, &val64);
	assert(res == 0);
	if (val64 != 0) {
		fprintf(stderr, "tv mismatch: expected %u, got %" PRIu64 "\n",
								0, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_STREAM_ID, &val64);
	assert(res == 0);
	if (val64 != STREAM_ID) {
		fprintf(stderr, "Stream ID mismatch: expected %" PRIu64
				", got %" PRIu64 "\n", STREAM_ID, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_SEQ_NUM, &val64);
	assert(res == 0);

	if (val64 != expected_seq) {
		/* If we have a sequence number mismatch, we simply log the
		 * issue and continue to process the packet. We don't want to
		 * invalidate it since it is a valid packet after all.
		 */
		fprintf(stderr, "Sequence number mismatch: expected %u, got %"
				PRIu64 "\n", expected_seq, val64);
		expected_seq = val64;
	}

	expected_seq++;

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_STREAM_DATA_LEN,
									&val64);
	assert(res == 0);
	if (val64 != STREAM_DATA_LEN) {
		fprintf(stderr, "Data len mismatch: expected %lu, got %"
					PRIu64 "\n", STREAM_DATA_LEN, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_TAG, &val64);
	assert(res == 0);
	if (val64 != AVTP_IECIIDC_TAG_CIP) {
		fprintf(stderr, "tag mismatch: expected %u, got %" PRIu64 "\n",
						AVTP_IECIIDC_TAG_CIP, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CHANNEL, &val64);
	assert(res == 0);
	if (val64 != 31) {
		fprintf(stderr, "channel mismatch: expected %u, got %" PRIu64
							"\n", 31, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_SID, &val64);
	assert(res == 0);
	if (val64 != 63) {
		fprintf(stderr, "sid mismatch: expected %u, got %" PRIu64 "\n",
								63, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_DBS, &val64);
	assert(res == 0);
	if (val64 != 6) {
		fprintf(stderr, "dbs mismatch: expected %u, got %" PRIu64 "\n",
								6, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_FN, &val64);
	assert(res == 0);
	if (val64 != 3) {
		fprintf(stderr, "fn mismatch: expected %u, got %" PRIu64 "\n",
								3, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_QPC, &val64);
	assert(res == 0);
	if (val64 != 0) {
		fprintf(stderr, "GV mismatch: expected %u, got %" PRIu64 "\n",
								0, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_SPH, &val64);
	assert(res == 0);
	if (val64 != 1) {
		fprintf(stderr, "sph mismatch: expected %u, got %" PRIu64 "\n",
								1, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_FMT, &val64);
	assert(res == 0);
	if (val64 != 32) {
		fprintf(stderr, "fmt mismatch: expected %u, got %" PRIu64 "\n",
								32, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_TSF, &val64);
	assert(res == 0);
	if (val64 != 0) {
		fprintf(stderr, "tsf mismatch: expected %u, got %" PRIu64 "\n",
								0, val64);
		return false;
	}

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_DBC, &val64);
	assert(res == 0);
	if (val64 != expected_dbc) {
		/* As with sequence mismatch, we'll not discard this packet,
		 * only log that the mismatch happened */
		fprintf(stderr, "dbc mismatch: expected %u, got %" PRIu64 "\n",
							expected_dbc, val64);
	}
	expected_dbc += 8;

	return true;
}

static int new_packet(int sk_fd, int timer_fd)
{
	int res;
	ssize_t n;
	uint32_t avtp_time;
	struct timespec tspec;
	struct avtp_stream_pdu *pdu = alloca(PDU_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) pdu->avtp_payload;
	struct avtp_ieciidc_cip_source_packet *sp =
		(struct avtp_ieciidc_cip_source_packet *) pay->cip_data_payload;

	memset(pdu, 0, PDU_SIZE);

	n = recv(sk_fd, pdu, PDU_SIZE, 0);
	if (n < 0 || n != PDU_SIZE) {
		perror("Failed to receive data");
		return -1;
	}

	if (!is_valid_packet(pdu)) {
		fprintf(stderr, "Dropping packet\n");
		return 0;
	}

	/* There are no helpers for payload fields, so one must remember
	 * of byte ordering */
	avtp_time = ntohl(sp->avtp_source_packet_header_timestamp);

	res = get_presentation_time(avtp_time, &tspec);
	if (res < 0)
		return -1;

	res = schedule_packet(timer_fd, &tspec, sp->cip_with_sph_payload);
	if (res < 0)
		return -1;

	return 0;
}

static int timeout(int fd)
{
	int res;
	ssize_t n;
	uint64_t expirations;
	struct packet_entry *entry;

	n = read(fd, &expirations, sizeof(uint64_t));
	if (n < 0) {
		perror("Failed to read timerfd");
		return -1;
	}

	assert(expirations == 1);

	entry = STAILQ_FIRST(&packets);
	assert(entry != NULL);

	res = present_data(entry->mpegts_packet, MPEG_TS_PACKET_LEN);
	if (res < 0)
		return -1;

	STAILQ_REMOVE_HEAD(&packets, entries);
	free(entry);

	if (!STAILQ_EMPTY(&packets)) {
		entry = STAILQ_FIRST(&packets);

		res = arm_timer(fd, &entry->tspec);
		if (res < 0)
			return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int sk_fd, timer_fd, res;
	struct pollfd fds[2];

	argp_parse(&argp, argc, argv, 0, NULL, NULL);

	STAILQ_INIT(&packets);

	sk_fd = create_listener_socket(ifname, macaddr, ETH_P_TSN);
	if (sk_fd < 0)
		return 1;

	timer_fd = timerfd_create(CLOCK_REALTIME, 0);
	if (timer_fd < 0) {
		close(sk_fd);
		return 1;
	}

	fds[0].fd = sk_fd;
	fds[0].events = POLLIN;
	fds[1].fd = timer_fd;
	fds[1].events = POLLIN;

	while (1) {
		res = poll(fds, 2, -1);
		if (res < 0) {
			perror("Failed to poll() fds");
			goto err;
		}

		if (fds[0].revents & POLLIN) {
			res = new_packet(sk_fd, timer_fd);
			if (res < 0)
				goto err;
		}

		if (fds[1].revents & POLLIN) {
			res = timeout(timer_fd);
			if (res < 0)
				goto err;
		}
	}

	return 0;

err:
	close(sk_fd);
	close(timer_fd);
	return 1;
}
