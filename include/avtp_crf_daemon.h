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

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AVTP_CRF_DMN_SOCKET_NAME	"/tmp/crf"

typedef enum {
	AVTP_CRF_DMN_REQ_REGISTER,
} avtp_crf_daemon_request_type_t;

typedef enum {
	AVTP_CRF_DMN_EVT_PKT_REVC,
	AVTP_CRF_DMN_EVT_RESENT_TIME,
} avtp_crf_daemon_event_type_t;

struct avtp_crf_daemon_req {
	avtp_crf_daemon_request_type_t type;
	union {
		struct {
			uint32_t events_per_sec;
			avtp_crf_daemon_event_type_t event_type;
		} reg;
	};
} __attribute__ ((__packed__));


typedef enum {
	AVTP_CRF_DMN_RESP_ERR,
	AVTP_CRF_DMN_RESP_EVT,
} avtp_crf_daemon_response_type_t;

struct avtp_crf_daemon_resp {
	avtp_crf_daemon_response_type_t type;
	union {
		struct {
			int err;
		} err;
		struct {
			uint64_t timestamp;
		} evt;
	};
} __attribute__ ((__packed__));


int avtp_crf_daemon_connect(const char socket_name[]);

#ifdef __cplusplus
}
#endif
