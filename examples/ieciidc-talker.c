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

/* IEC 61883/IIDC Talker example.
 *
 * This example implements a very simple IEC 61883 talker application which
 * reads an MPEG-TS stream from stdin, creates AVTP IEC 61883/IIDC packets and
 * transmits them via the network.
 *
 * For simplicity, the example supports only MPEG-TS streams, and only one
 * source packet is packed into each AVTP packet sent.
 *
 * TSN stream parameters (e.g. destination mac address, traffic priority) are
 * passed via command-line arguments. Run 'ieciidc-talker --help' for more
 * information.
 *
 * In order to have this example working properly, make sure you have
 * configured FQTSS feature from your NIC according (for further information
 * see tc-cbs(8)). Also, this example relies on system clock to set the AVTP
 * timestamp so make sure it is synchronized with the PTP Hardware Clock (PHC)
 * from your NIC and that the PHC is synchronized with the network clock. For
 * further information see ptp4l(8) and phc2sys(8).
 *
 * The easiest way to use this example is by combining it with a GStreamer
 * pipeline. We use GStreamer to provide an MPEG-TS stream that is sent to
 * stdout, from where this example reads the stream. So, to generate
 * an MPEG-TS video to send via TSN network, you can do something like:
 *
 * $ gst-launch-1.0 -e -q videotestsrc pattern=ball ! x264enc
 *  ! mpegtsmux ! filesink location=/dev/stdout
 *  | ieciidc-talker <args>
 *
 */

#include <alloca.h>
#include <argp.h>
#include <arpa/inet.h>
#include <assert.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "avtp.h"
#include "avtp_ieciidc.h"
#include "examples/common.h"

#define STREAM_ID		0xAABBCCDDEEFF0001
#define MPEG_TS_PACKET_LEN	188

#define DATA_LEN		(MPEG_TS_PACKET_LEN + sizeof(uint32_t)) /* MPEG-TS size + SPH */
#define CIP_HEADER_LEN		(sizeof(uint32_t) * 2)
#define STREAM_DATA_LEN		DATA_LEN + CIP_HEADER_LEN
#define PDU_SIZE		(sizeof(struct avtp_stream_pdu) + CIP_HEADER_LEN + DATA_LEN)

static char ifname[IFNAMSIZ];
static uint8_t macaddr[ETH_ALEN];
static int priority = -1;
static int max_transit_time;

static struct argp_option options[] = {
	{"dst-addr", 'd', "MACADDR", 0, "Stream Destination MAC address" },
	{"ifname", 'i', "IFNAME", 0, "Network Interface" },
	{"max-transit-time", 'm', "MSEC", 0, "Maximum Transit Time in ms" },
	{"prio", 'p', "NUM", 0, "SO_PRIORITY to be set in socket" },
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
	case 'm':
		max_transit_time = atoi(arg);
		break;
	case 'p':
		priority = atoi(arg);
		break;
	}

	return 0;
}

static struct argp argp = { options, parser };

static void init_pdu(struct avtp_stream_pdu *pdu)
{
	int res;

	res = avtp_ieciidc_pdu_init(pdu, AVTP_IECIIDC_TAG_CIP);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_TV, 0);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_STREAM_ID,
								STREAM_ID);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu,
			AVTP_IECIIDC_FIELD_STREAM_DATA_LEN, STREAM_DATA_LEN);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_GV, 0);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_GATEWAY_INFO, 0);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CHANNEL, 31);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_QI_1, 0);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_SID, 63);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_DBS, 6);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_FN, 3);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_QPC, 0);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_SPH, 1);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_QI_2, 2);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_FMT, 32);
	assert(res == 0);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_TSF, 0);
	assert(res == 0);
}

int main(int argc, char *argv[])
{
	int fd, res;
	struct sockaddr_ll sk_addr;
	struct avtp_stream_pdu *pdu = alloca(PDU_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *)pdu->avtp_payload;
	uint8_t seq_num = 0;
	uint8_t dbc = 0;

	argp_parse(&argp, argc, argv, 0, NULL, NULL);

	fd = create_talker_socket(priority);
	if (fd < 0)
		return 1;

	res = setup_socket_address(fd, ifname, macaddr, ETH_P_TSN, &sk_addr);
	if (res < 0)
		goto err;

	init_pdu(pdu);

	while (1) {
		ssize_t n;
		uint32_t avtp_time;
		struct avtp_ieciidc_cip_source_packet *sp;

		memset(pay->cip_data_payload, 0, DATA_LEN);
		sp = (struct avtp_ieciidc_cip_source_packet *)
							pay->cip_data_payload;

		n = read(STDIN_FILENO, sp->cip_with_sph_payload,
							MPEG_TS_PACKET_LEN);
		if (n == 0)
			break;

		if (n != MPEG_TS_PACKET_LEN) {
			fprintf(stderr, "read %zd bytes, expected %d\n", n,
							MPEG_TS_PACKET_LEN);
		}

		res = calculate_avtp_time(&avtp_time, max_transit_time);
		if (res < 0) {
			fprintf(stderr, "Failed to calculate avtp time\n");
			goto err;
		}

		/* There are no helpers for payload fields, so one must remember
		 * of byte ordering
		 */
		sp->avtp_source_packet_header_timestamp = htonl(avtp_time);

		res = avtp_ieciidc_pdu_set(pdu,
					AVTP_IECIIDC_FIELD_SEQ_NUM, seq_num++);
		assert(res == 0);

		res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_DBC,
									dbc);
		assert(res == 0);
		/* We just send one MPEG-TS packet per AVTP packet, so we
		 * increase dbc by the number of blocks on one MPEG-TS packet
		 */
		dbc += 8;

		n = sendto(fd, pdu, PDU_SIZE, 0, (struct sockaddr *) &sk_addr,
							sizeof(sk_addr));
		if (n < 0) {
			perror("Failed to send data");
			goto err;
		}

		if (n != PDU_SIZE) {
			fprintf(stderr, "wrote %zd bytes, expected %zd\n", n,
								PDU_SIZE);
		}
	}

	close(fd);
	return 0;

err:
	close(fd);
	return 1;
}
