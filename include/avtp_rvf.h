/*
 * Copyright (c) 2021, Fastree3D
 * Adrian Fiergolski <Adrian.Fiergolski@fastree3d.com>
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

/* RVF 'pixel depth' field values. */
#define AVTP_RVF_PIXEL_DEPTH_8	  0x01
#define AVTP_RVF_PIXEL_DEPTH_10	  0x02
#define AVTP_RVF_PIXEL_DEPTH_12	  0x03
#define AVTP_RVF_PIXEL_DEPTH_16	  0x04
#define AVTP_RVF_PIXEL_DEPTH_USER 0x0F

/* RVF 'pixel format' field values. */
#define AVTP_RVF_PIXEL_FORMAT_MONO	 0x00
#define AVTP_RVF_PIXEL_FORMAT_411	 0x01
#define AVTP_RVF_PIXEL_FORMAT_420	 0x02
#define AVTP_RVF_PIXEL_FORMAT_422	 0x03
#define AVTP_RVF_PIXEL_FORMAT_444	 0x04
#define AVTP_RVF_PIXEL_FORMAT_4224	 0x06
#define AVTP_RVF_PIXEL_FORMAT_4444	 0x07
#define AVTP_RVF_PIXEL_FORMAT_BAYER_GRBG 0x08
#define AVTP_RVF_PIXEL_FORMAT_BAYER_RGGB 0x09
#define AVTP_RVF_PIXEL_FORMAT_BAYER_BGGR 0x0A
#define AVTP_RVF_PIXEL_FORMAT_BAYER_GBRG 0x0B
#define AVTP_RVF_PIXEL_FORMAT_USER	 0x0F

/* RVF 'frame rate' field values. */
#define AVTP_RVF_FRAME_RATE_1	 0x01
#define AVTP_RVF_FRAME_RATE_2	 0x02
#define AVTP_RVF_FRAME_RATE_5	 0x03
#define AVTP_RVF_FRAME_RATE_10	 0x10
#define AVTP_RVF_FRAME_RATE_15	 0x11
#define AVTP_RVF_FRAME_RATE_20	 0x12
#define AVTP_RVF_FRAME_RATE_24	 0x13
#define AVTP_RVF_FRAME_RATE_25	 0x14
#define AVTP_RVF_FRAME_RATE_30	 0x15
#define AVTP_RVF_FRAME_RATE_48	 0x16
#define AVTP_RVF_FRAME_RATE_50	 0x17
#define AVTP_RVF_FRAME_RATE_60	 0x18
#define AVTP_RVF_FRAME_RATE_72	 0x19
#define AVTP_RVF_FRAME_RATE_85	 0x1A
#define AVTP_RVF_FRAME_RATE_100	 0x30
#define AVTP_RVF_FRAME_RATE_120	 0x31
#define AVTP_RVF_FRAME_RATE_150	 0x32
#define AVTP_RVF_FRAME_RATE_200	 0x33
#define AVTP_RVF_FRAME_RATE_240	 0x34
#define AVTP_RVF_FRAME_RATE_300	 0x35
#define AVTP_RVF_FRAME_RATE_USER 0x0F

/* RVF 'colorspace' field values. */
#define AVTP_RVF_COLORSPACE_YCbCr  0x01
#define AVTP_RVF_COLORSPACE_SRGB   0x02
#define AVTP_RVF_COLORSPACE_YCgCo  0x03
#define AVTP_RVF_COLORSPACE_GRAY   0x04
#define AVTP_RVF_COLORSPACE_XYZ	   0x05
#define AVTP_RVF_COLORSPACE_YCM	   0x06
#define AVTP_RVF_COLORSPACE_BT_601 0x07
#define AVTP_RVF_COLORSPACE_BT_709 0x08
#define AVTP_RVF_COLORSPACE_ITU_BT 0x09
#define AVTP_RVF_COLORSPACE_USER   0x0F

enum avtp_rvf_field {
	AVTP_RVF_FIELD_SV,
	AVTP_RVF_FIELD_MR,
	AVTP_RVF_FIELD_TV,
	AVTP_RVF_FIELD_SEQ_NUM,
	AVTP_RVF_FIELD_TU,
	AVTP_RVF_FIELD_STREAM_ID,
	AVTP_RVF_FIELD_TIMESTAMP,
	AVTP_RVF_FIELD_STREAM_DATA_LEN,
	AVTP_RVF_FIELD_ACTIVE_PIXELS,
	AVTP_RVF_FIELD_TOTAL_LINES,
	AVTP_RVF_FIELD_AP,
	AVTP_RVF_FIELD_F,
	AVTP_RVF_FIELD_EF,
	AVTP_RVF_FIELD_EVT,
	AVTP_RVF_FIELD_PD,
	AVTP_RVF_FIELD_I,
	AVTP_RVF_FIELD_RAW_PIXEL_DEPTH,
	AVTP_RVF_FIELD_RAW_PIXEL_FORMAT,
	AVTP_RVF_FIELD_RAW_FRAME_RATE,
	AVTP_RVF_FIELD_RAW_COLORSPACE,
	AVTP_RVF_FIELD_RAW_NUM_LINES,
	AVTP_RVF_FIELD_RAW_I_SEQ_NUM,
	AVTP_RVF_FIELD_RAW_LINE_NUMBER,
	AVTP_RVF_FIELD_MAX,
};

struct avtp_rvf_payload {
	uint64_t raw_header;

	uint8_t raw_data[0];
} __attribute__((__packed__));

/* Get value of RVF AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be retrieved.
 * @val: Pointer to variable which the retrieved value should be saved.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_rvf_pdu_get(const struct avtp_stream_pdu *pdu,
		     enum avtp_rvf_field field, uint64_t *val);

/* Set value of RVF AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be set.
 * @val: Value to be set.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_rvf_pdu_set(struct avtp_stream_pdu *pdu, enum avtp_rvf_field field,
		     uint64_t val);

/* Initialize RVF AVTPDU. All AVTPDU fields are initialized with zero except
 * 'subtype' (which is set to AVTP_SUBTYPE_RVF), 'sv' (which is set to 1),
 * @pdu: Pointer to PDU struct.
 * @subtype: AVTP CVF Format Subtype of this AVTPDU.
 *
 * Return values:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_rvf_pdu_init(struct avtp_stream_pdu *pdu);

#ifdef __cplusplus
}
#endif
