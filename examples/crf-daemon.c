/*
 * Copyright (c) 2018, Intel Corporation
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <argp.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <errno.h>

#include "avtp.h"
#include "avtp_crf.h"
#include "avtp_crf_daemon.h"
#include "examples/common.h"

#define NSEC_PER_SEC		1000000000ULL

#define CRF_STREAM_ID		0xAABBCCDDEEFF0002
/* Values based on Spec 1722 Table 28 recommendation. */
#define CRF_SAMPLE_RATE		48000
#define CRF_TIMESTAMPS_PER_SEC	300
#define MCLKLIST_TS_PER_CRF	(CRF_SAMPLE_RATE / CRF_TIMESTAMPS_PER_SEC)
#define MCLK_PERIOD		(NSEC_PER_SEC / CRF_TIMESTAMPS_PER_SEC)
#define TIMESTAMPS_PER_PKT	6
#define CRF_DATA_LEN		(sizeof(uint64_t) * TIMESTAMPS_PER_PKT)
#define CRF_PDU_SIZE		(sizeof(struct avtp_crf_pdu) + CRF_DATA_LEN)


#define ARRAY_SIZE(a)	( sizeof(a) / sizeof((a)[0]) )

static const size_t MAX_CLIENTS = 128;

typedef struct {
	int fd;
	uint32_t events_per_crf;
	avtp_crf_daemon_event_type_t event_type;

} crf_daemon_client_t;

static uint8_t crf_seq_num;
static char ifname[IFNAMSIZ];
static uint8_t crf_macaddr[ETH_ALEN];

static const struct argp_option options[] = {
	{"crf-addr", 'c', "MACADDR", 0, "CRF Stream Destination MAC address" },
	{"ifname", 'i', "IFNAME", 0, "Network Interface" },
	{ 0 }
};

static error_t parser(int key, char *arg, struct argp_state *state)
{
	int res;

	switch (key) {
	case 'c':
		res = sscanf(arg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&crf_macaddr[0], &crf_macaddr[1], &crf_macaddr[2],
			&crf_macaddr[3], &crf_macaddr[4], &crf_macaddr[5]);
		if (res != 6) {
			fprintf(stderr, "Invalid CRF address\n");
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


static void client_close(struct pollfd* const pfd, crf_daemon_client_t* const client)
{
	close(client->fd);
	client->fd = -1;
	pfd->fd = -1;
}

static int mclk_enqueue_ts(const int fd, const uint64_t timestamp)
{
	const struct avtp_crf_daemon_resp event = {
		.type = AVTP_CRF_DMN_RESP_EVT,
		.evt.timestamp = timestamp,
	};
	const ssize_t rc = send(fd, &event, sizeof(event), 0);
	if (rc < 0) {
		perror("send() failed");
		return -errno;
	} else if (rc != sizeof(event)) {
		printf("Sent only %ld bytes\n", rc);
		return -EPIPE;
	}

	return 0;
}

/* This routine generates media clock timestamps using timestamps from CRF
 * stream.
 */
static int recover_mclk(struct avtp_crf_pdu *pdu, const int fd)
{
	int res, idx;
	uint64_t ts_mclk, ts_crf;

	/* To recover the media clock we consider only the first timestamp from
	 * CRF PDU since the others timestamps are incremented monotonically
	 * from the first timestamp (see Section 10.7 from IEEE 1722-2016
	 * spec).
	 */
	ts_crf = be64toh(pdu->crf_data[0]);

	for (idx = 0; idx < MCLKLIST_TS_PER_CRF; idx++) {
		ts_mclk = ts_crf + (idx * MCLK_PERIOD);
		res = mclk_enqueue_ts(fd, ts_mclk);
		if (res < 0)
			return res;
	}

	return 0;
}

static bool is_valid_crf_pdu(struct avtp_crf_pdu *pdu)
{
	int res;
	uint32_t val32;
	uint64_t val64;
	struct avtp_common_pdu *common = (struct avtp_common_pdu *) pdu;

	res = avtp_pdu_get(common, AVTP_FIELD_SUBTYPE, &val32);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF subtype field: %d\n", res);
		return false;
	}
	if (val32 != AVTP_SUBTYPE_CRF)
		return false;

	res = avtp_pdu_get(common, AVTP_FIELD_VERSION, &val32);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF version field: %d\n", res);
		return false;
	}
	if (val32 != 0) {
		fprintf(stderr, "CRF: Version mismatch: expected %u, got %u\n",
								0, val32);
		return false;
	}

	res = avtp_crf_pdu_get(pdu, AVTP_CRF_FIELD_SV, &val64);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF sv field: %d\n", res);
		return false;
	}
	if (val64 != 1) {
		fprintf(stderr, "CRF: sv mismatch: expected %u, got %" PRIu64 "\n",
								1, val64);
		return false;
	}

	res = avtp_crf_pdu_get(pdu, AVTP_CRF_FIELD_FS, &val64);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF fs field: %d\n", res);
		return false;
	}
	if (val64 != 0) {
		fprintf(stderr, "CRF: fs mismatch: expected %u, got %" PRIu64 "\n",
								0, val64);
		return false;
	}

	res = avtp_crf_pdu_get(pdu, AVTP_CRF_FIELD_SEQ_NUM, &val64);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF sequence num field: %d\n",
									res);
		return false;
	}
	if (val64 != crf_seq_num) {
		/* If we have a sequence number mismatch, we simply log the
		 * issue and continue to process the packet. We don't want to
		 * invalidate it since it is a valid packet after all.
		 */
		fprintf(stderr, "CRF: Sequence number mismatch: expected %u, got %" PRIu64 "\n",
							crf_seq_num, val64);

		crf_seq_num = val64;
	}

	crf_seq_num++;

	res = avtp_crf_pdu_get(pdu, AVTP_CRF_FIELD_TYPE, &val64);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF format field: %d\n", res);
		return false;
	}
	if (val64 != AVTP_CRF_TYPE_AUDIO_SAMPLE) {
		fprintf(stderr, "CRF: Format mismatch: expected %u, got %" PRIu64 "\n",
					AVTP_CRF_TYPE_AUDIO_SAMPLE, val64);
		return false;
	}

	res = avtp_crf_pdu_get(pdu, AVTP_CRF_FIELD_STREAM_ID, &val64);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF stream ID field: %d\n",
									res);
		return false;
	}
	if (val64 != CRF_STREAM_ID) {
		fprintf(stderr, "CRF: Stream ID mismatch: expected %" PRIu64 ", got %" PRIu64 "\n",
							CRF_STREAM_ID, val64);
		return false;
	}

	res = avtp_crf_pdu_get(pdu, AVTP_CRF_FIELD_PULL, &val64);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF multiplier modifier field: %d\n",
									res);
		return false;
	}
	if (val64 != AVTP_CRF_PULL_MULT_BY_1) {
		fprintf(stderr, "CRF Pull mismatch: expected %u, got %" PRIu64 "\n",
					AVTP_CRF_PULL_MULT_BY_1, val64);
		return false;
	}

	res = avtp_crf_pdu_get(pdu, AVTP_CRF_FIELD_BASE_FREQ, &val64);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF base frequency field: %d\n",
									res);
		return false;
	}
	if (val64 != CRF_SAMPLE_RATE) {
		fprintf(stderr, "CRF Base frequency: expected %u, got %" PRIu64 "\n",
						CRF_SAMPLE_RATE, val64);
		return false;
	}

	res = avtp_crf_pdu_get(pdu, AVTP_CRF_FIELD_CRF_DATA_LEN, &val64);
	if (res < 0) {
		fprintf(stderr, "Failed to get CRF data length field: %d\n",
									res);
		return false;
	}
	if (val64 != CRF_DATA_LEN) {
		fprintf(stderr, "CRF Data length mismatch: expected %zu, got %" PRIu64 "\n",
							CRF_DATA_LEN, val64);
		return false;
	}

	return true;
}

static int handle_crf_pdu(struct avtp_crf_pdu *pdu, struct pollfd fds[], crf_daemon_client_t clients[], const int clients_max)
{
	if (!is_valid_crf_pdu(pdu))
		return 0;

	int ret = 0;
	for (int i=0; i<clients_max; i++) {
		crf_daemon_client_t* client = &clients[i];
		if (client->fd < 0)
			continue;
		if (recover_mclk(pdu, client->fd) < 0) {
			ret = -client->fd;
			client_close(&fds[i], client);
		}
	}

	return ret;
}

static int process_crf(const int crf_fd, struct pollfd fds[], crf_daemon_client_t clients[],
		       const int clients_max)
{
	ssize_t n;
	struct avtp_crf_pdu *pdu = alloca(CRF_PDU_SIZE);

	memset(pdu, 0, CRF_PDU_SIZE);

	n = recv(crf_fd, pdu, CRF_PDU_SIZE, 0);
	if (n < 0) {
		perror("Failed to receive data");
		return -1;
	}

	/* The protocol type from rx socket is set to ETH_P_ALL so we receive
	 * non-AVTP packets as well. In order to filter out those packets, we
	 * check the number of bytes received. If it doesn't match the CRF pdu
	 * size we drop the packet.
	 */
	if (n != CRF_PDU_SIZE)
		return 0;

	return handle_crf_pdu(pdu, fds, clients, clients_max);
}


static int process_request(const int fd, crf_daemon_client_t* const client)
{
	struct avtp_crf_daemon_req req;
	ssize_t rc = -1;
	while ( (rc = recv(fd, &req, sizeof(req), 0)) == sizeof(req) ) {
		switch (req.type) {
		case AVTP_CRF_DMN_REQ_REGISTER:
			client->fd = fd;
			client->events_per_crf = req.reg.events_per_sec; // TODO convert value depending on CRF package information
			client->event_type = req.reg.event_type;
			break;
		default:
			printf("Client request %d not supported", req.type);
			break;
		}
	}

	int ret = 0;
	if (rc < 0) {
		/* check if no data available */
		if (errno != EWOULDBLOCK && errno != EAGAIN) {
			perror("recv() failed");
			ret = -errno;
		}
	} else if (rc == 0) {
		printf("Connection closed\n");
		ret = -EPIPE;
	} else {
		/* the size of the received package was wrong */
		printf("Wrong package size %ld\n", rc);
		ret = -EINVAL;
	}

	if (ret < 0)
		client->fd = -1;
	return ret;
}

int main (int argc, char *argv[])
{
	argp_parse(&argp, argc, argv, 0, NULL, NULL);


	/* In case this example is running on the same host where crf-talker is
	 * running, we set protocol type to ETH_P_ALL to allow CRF traffic to
	 * loop back.
	 */
	const int crf_fd = create_listener_socket(ifname, crf_macaddr, ETH_P_ALL);
	if (crf_fd < 0) {
		perror("Failed to open socket");
		return -1;
	}

	const int server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket() failed");
		return -1;
	}

	/* in case the socket was not close correctly allow to reuse it */
	const int on = 1;
	if (setsockopt(server_fd, SOL_SOCKET,  SO_REUSEADDR, (const char *)&on, sizeof(on))< 0) {
		perror("setsockopt() failed");
		close(server_fd);
		return -1;
	}

	/* Set socket to be nonblocking. All of the sockets for the incoming connections will also
	 * be nonblocking since they will inherit that state from the listening socket.
	 */
	if(ioctl(server_fd, FIONBIO, (const char *)&on) < 0) {
		perror("ioctl() failed");
		close(server_fd);
		return -1;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_LOCAL;
	strncpy(addr.sun_path, AVTP_CRF_DMN_SOCKET_NAME, sizeof(addr.sun_path) - 1);
	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind() failed");
		close(server_fd);
		return -1;
	}

	if (listen(server_fd, 32) < 0) {
		perror("listen() failed");
		close(server_fd);
		return -1;
	}

	static const size_t EXTRA_FDS = 2;
	struct pollfd fds[EXTRA_FDS + MAX_CLIENTS];
	memset(fds, 0 , sizeof(fds));
	for (size_t i=0; i<ARRAY_SIZE(fds); i++)
		fds[i].fd = -1;
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;
	fds[1].fd = crf_fd;
	fds[1].events = POLLIN;
	nfds_t nfds = EXTRA_FDS;

	crf_daemon_client_t clients[MAX_CLIENTS];
	for (int i=0; i<ARRAY_SIZE(clients); i++)
		clients[i].fd = -1;

	int rc = -1;
	while ( (rc = poll(fds, nfds, -1)) > 0) {
		const nfds_t current_size = nfds;
		for (nfds_t i = 0; i < current_size; i++) {
			if(fds[i].revents == 0)
				continue;
			else if(fds[i].revents != POLLIN) {
				printf("Error! fds[%ld].revents = %x\n", i, fds[i].revents);
				if (i < EXTRA_FDS) {
					/* an important FD had an issue. Therefore shuting down
					 * the daemon
					 */
					rc = -1;
					break;
				} else {
					const int client_id = i - EXTRA_FDS;
					client_close(&fds[i], &clients[client_id]);
				}
			} else if (fds[i].fd == server_fd) {
				int new_sd = -1;
				while ( (new_sd = accept(server_fd, NULL, NULL)) >= 0) {
					/* search for free space */
					for (nfds_t k=0; k<ARRAY_SIZE(fds); k++) {
						if (fds[k].fd < 0) {
							fds[k].fd = new_sd;
							fds[k].events = POLLIN;
							if (k >= nfds)
								nfds = k + 1;
							break;
						}
					}
				}
				if (new_sd < 0 && errno != EWOULDBLOCK) {
					perror("accept() failed");
				}
			} else if (fds[i].fd == crf_fd) {
				const int clients_max = nfds - EXTRA_FDS;
				process_crf(crf_fd, &fds[EXTRA_FDS], clients, clients_max);
			} else {
				crf_daemon_client_t* const client = &clients[i - EXTRA_FDS];
				if (process_request(fds[i].fd, client) < 0) {
					close(fds[i].fd);
					fds[i].fd = -1;
				}
			}
		}
	}
	if (rc < 0) {
		perror("poll() failed");
	} else if (rc == 0) {
		printf("poll() timed out.  End program.\n");
	}

	for (nfds_t i = 0; i < nfds; i++) {
		if(fds[i].fd >= 0)
			close(fds[i].fd);
	}

	return 0;
}
