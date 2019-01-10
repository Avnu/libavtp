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

#include <stdint.h>

/* Calculate AVTP presentation time based on current time and informed
 * max_transit_time.
 * @avtp_time: Pointer to variable which the calculated time should be saved.
 * @max_transit_time: Max transit time for the network
 *
 * Returns:
 *    0: Success.
 *    -1: If could not get current time.
 */
int calculate_avtp_time(uint32_t *avtp_time, uint32_t max_transit_time);

/* Given an AVTP presentation time, retrieve correspondent time on
 * CLOCK_REALTIME.
 * @avtp_time: AVTP presentation time to be converted.
 * @ts: Pointer to struct timespec where obtained time should be saved.
 *
 * Returns:
 *    0: Success.
 *    -1: If could not get CLOCK_REALTIME.
 */
int get_presentation_time(uint64_t avtp_time, struct timespec *tspec);

/* Create TSN socket to listen for incomimg packets.
 * @ifname: Network interface name where to create the socket.
 * @macaddr: Stream destination MAC address.
 * @protocol: Protocol to listen to.
 *
 * Returns:
 *    >= 0: Socket file descriptor. Should be closed with close() when done.
 *    -1: Could not create socket.
 */
int create_listener_socket(char *ifname, uint8_t macaddr[], int protocol);

/* Create TSN socket to send packets.
 * @priority: SO_PRIORITY to be set in socket.
 *
 * Returns:
 *    >= 0: Socket file descriptor. Should be closed with close() when done.
 *    -1: Could not create socket.
 */
int create_talker_socket(int priority);

/* Set struct sockaddr_ll with TSN and socket parameters, so it can be used
 * later on sendo() or bind() calls.
 * @fd: Socket file descriptor.
 * @ifname: Network interface name where to create the socket.
 * @macaddr: Stream destination MAC address.
 * @sk_addr: Pointer to struct sockaddr_ll to be set up.
 * @protocol: Protocol used.
 *
 * Returns:
 *    0: Success.
 *    -1: Could not get interface index.
 */
int setup_socket_address(int fd, const char *ifname, uint8_t macaddr[],
				int protocol, struct sockaddr_ll *sk_addr);

/* Write data to standard output.
 * @data: Data to be written.
 * @len: Number of bytes to be written.
 *
 * Returns:
 *    0: Success. It's only reported when all data is successfully written.
 *    -1: Could not write all data.
 */
int present_data(uint8_t *data, size_t len);

/* Arm a timerfd to go off on informed time.
 * @fd: File descriptor of the timer.
 * @tspec: When the time should go off.
 *
 * Returns:
 *    0: Success.
 *    -1: Could not arm timer.
 */
int arm_timer(int fd, struct timespec *tspec);
