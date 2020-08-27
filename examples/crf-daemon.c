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
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <errno.h>

#include "avtp_crf_daemon.h"

#define ARRAY_SIZE(a)	( sizeof(a) / sizeof((a)[0]) )

static const size_t MAX_CLIENTS = 128;

typedef struct {
	int fd;
	uint32_t events_per_crf;
	avtp_crf_daemon_event_type_t event_type;

} crf_daemon_client_t;


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
