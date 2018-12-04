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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CVF 'format' field values. */
#define AVTP_CVF_FORMAT_RFC			0x02

/* CVF 'format subtype' field values. */
#define AVTP_CVF_FORMAT_SUBTYPE_MJPEG		0x00
#define AVTP_CVF_FORMAT_SUBTYPE_H264		0x01
#define AVTP_CVF_FORMAT_SUBTYPE_JPEG2000	0x02

enum avtp_cvf_field {
	AVTP_CVF_FIELD_SV,
	AVTP_CVF_FIELD_MR,
	AVTP_CVF_FIELD_TV,
	AVTP_CVF_FIELD_SEQ_NUM,
	AVTP_CVF_FIELD_TU,
	AVTP_CVF_FIELD_STREAM_ID,
	AVTP_CVF_FIELD_TIMESTAMP,
	AVTP_CVF_FIELD_STREAM_DATA_LEN,
	AVTP_CVF_FIELD_FORMAT,
	AVTP_CVF_FIELD_FORMAT_SUBTYPE,
	AVTP_CVF_FIELD_M,
	AVTP_CVF_FIELD_EVT,
	AVTP_CVF_FIELD_H264_PTV,
	AVTP_CVF_FIELD_H264_TIMESTAMP,
	AVTP_CVF_FIELD_MAX,
};

struct avtp_cvf_h264_payload {
	uint32_t h264_header;
	uint8_t h264_data[0];
} __attribute__((__packed__));

/* Get value of CVF AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be retrieved.
 * @val: Pointer to variable which the retrieved value should be saved.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_cvf_pdu_get(const struct avtp_stream_pdu *pdu,
				enum avtp_cvf_field field, uint64_t *val);

/* Set value of CVF AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be set.
 * @val: Value to be set.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_cvf_pdu_set(struct avtp_stream_pdu *pdu, enum avtp_cvf_field field,
								uint64_t val);

/* Initialize CVF AVTPDU. All AVTPDU fields are initialized with zero except
 * 'subtype' (which is set to AVTP_SUBTYPE_CVF), 'sv' (which is set to 1),
 * 'format' (which is set to AVTP_CVF_FORMAT_RFC) and 'format_subtype'
 * (which is set to the `subtype` specified).
 * @pdu: Pointer to PDU struct.
 * @subtype: AVTP CVF Format Subtype of this AVTPDU.
 *
 * Return values:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_cvf_pdu_init(struct avtp_stream_pdu *pdu, uint8_t subtype);

#ifdef __cplusplus
}
#endif
