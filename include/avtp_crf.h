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

#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CRF 'type' field values. */
#define AVTP_CRF_TYPE_USER			0x00
#define AVTP_CRF_TYPE_AUDIO_SAMPLE		0x01
#define AVTP_CRF_TYPE_VIDEO_FRAME		0x02
#define AVTP_CRF_TYPE_VIDEO_LINE		0x03
#define AVTP_CRF_TYPE_MACHINE_CYCLE		0x04

/* CRF 'pull' field values. */
#define AVTP_CRF_PULL_MULT_BY_1			0x00
#define AVTP_CRF_PULL_MULT_BY_1_OVER_1_001	0x01
#define AVTP_CRF_PULL_MULT_BY_1_001		0x02
#define AVTP_CRF_PULL_MULT_BY_24_OVER_25	0x03
#define AVTP_CRF_PULL_MULT_BY_25_OVER_24	0x04
#define AVTP_CRF_PULL_MULT_BY_1_OVER_8		0x05

struct avtp_crf_pdu {
	uint32_t subtype_data;
	uint64_t stream_id;
	uint64_t packet_info;
	uint64_t crf_data[0];
} __attribute__ ((__packed__));

enum avtp_crf_field {
	AVTP_CRF_FIELD_SV,
	AVTP_CRF_FIELD_MR,
	AVTP_CRF_FIELD_FS,
	AVTP_CRF_FIELD_TU,
	AVTP_CRF_FIELD_SEQ_NUM,
	AVTP_CRF_FIELD_TYPE,
	AVTP_CRF_FIELD_STREAM_ID,
	AVTP_CRF_FIELD_PULL,
	AVTP_CRF_FIELD_BASE_FREQ,
	AVTP_CRF_FIELD_CRF_DATA_LEN,
	AVTP_CRF_FIELD_TIMESTAMP_INTERVAL,
	AVTP_CRF_FIELD_MAX,
};

/* Get value from CRF AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be retrieved.
 * @val: Pointer to variable which the retrieved value should be saved.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_crf_pdu_get(const struct avtp_crf_pdu *pdu,
				enum avtp_crf_field field, uint64_t *val);

/* Set value from CRF AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be set.
 * @val: Value to be set.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_crf_pdu_set(struct avtp_crf_pdu *pdu, enum avtp_crf_field field,
								uint64_t val);

/* Initialize CRF AVTPDU. All AVTPDU fields are initialized with zero except
 * 'subtype' (which is set to AVTP_SUBTYPE_CRF) and 'sv' (which is set to 1).
 * @pdu: Pointer to PDU struct.
 *
 * Return values:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_crf_pdu_init(struct avtp_crf_pdu *pdu);

#ifdef __cplusplus
}
#endif
