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

#include <arpa/inet.h>
#include <string.h>

#include "avtp.h"
#include "avtp_cvf.h"
#include "avtp_stream.h"
#include "util.h"

#define SHIFT_FORMAT		(31 - 7)
#define SHIFT_FORMAT_SUBTYPE	(31 - 15)
#define SHIFT_M			(31 - 19)
#define SHIFT_EVT		(31 - 23)
#define SHIFT_PTV		(31 - 18)

#define MASK_FORMAT		(BITMASK(8) << SHIFT_FORMAT)
#define MASK_FORMAT_SUBTYPE	(BITMASK(8) << SHIFT_FORMAT_SUBTYPE)
#define MASK_M			(BITMASK(1) << SHIFT_M)
#define MASK_EVT		(BITMASK(4) << SHIFT_EVT)
#define MASK_PTV		(BITMASK(1) << SHIFT_PTV)

static int get_field_value(const struct avtp_stream_pdu *pdu,
				enum avtp_cvf_field field, uint64_t *val)
{
	uint32_t bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_CVF_FIELD_FORMAT:
		mask = MASK_FORMAT;
		shift = SHIFT_FORMAT;
		bitmap = ntohl(pdu->format_specific);
		break;
	case AVTP_CVF_FIELD_FORMAT_SUBTYPE:
		mask = MASK_FORMAT_SUBTYPE;
		shift = SHIFT_FORMAT_SUBTYPE;
		bitmap = ntohl(pdu->format_specific);
		break;
	case AVTP_CVF_FIELD_M:
		mask = MASK_M;
		shift = SHIFT_M;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_CVF_FIELD_EVT:
		mask = MASK_EVT;
		shift = SHIFT_EVT;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_CVF_FIELD_H264_PTV:
		mask = MASK_PTV;
		shift = SHIFT_PTV;
		bitmap = ntohl(pdu->packet_info);
		break;
	default:
		return -EINVAL;
	}

	*val = BITMAP_GET_VALUE(bitmap, mask, shift);

	return 0;
}

int avtp_cvf_pdu_get(const struct avtp_stream_pdu *pdu,
				enum avtp_cvf_field field, uint64_t *val)
{
	int res;

	if (!pdu || !val)
		return -EINVAL;

	switch (field) {
	case AVTP_CVF_FIELD_SV:
	case AVTP_CVF_FIELD_MR:
	case AVTP_CVF_FIELD_TV:
	case AVTP_CVF_FIELD_SEQ_NUM:
	case AVTP_CVF_FIELD_TU:
	case AVTP_CVF_FIELD_STREAM_DATA_LEN:
	case AVTP_CVF_FIELD_TIMESTAMP:
	case AVTP_CVF_FIELD_STREAM_ID:
		res = avtp_stream_pdu_get(pdu, (enum avtp_stream_field) field,
									val);
		break;
	case AVTP_CVF_FIELD_FORMAT:
	case AVTP_CVF_FIELD_FORMAT_SUBTYPE:
	case AVTP_CVF_FIELD_M:
	case AVTP_CVF_FIELD_EVT:
	case AVTP_CVF_FIELD_H264_PTV:
		res = get_field_value(pdu, field, val);
		break;
	case AVTP_CVF_FIELD_H264_TIMESTAMP:
	{
		/* This field lives on H.264 header, inside avtp_payload */
		struct avtp_cvf_h264_payload *pay =
			(struct avtp_cvf_h264_payload *)pdu->avtp_payload;
		*val = ntohl(pay->h264_header);
		res = 0;
		break;
	}
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

static int set_field_value(struct avtp_stream_pdu *pdu,
				enum avtp_cvf_field field, uint32_t val)
{
	uint32_t *ptr, bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_CVF_FIELD_FORMAT:
		mask = MASK_FORMAT;
		shift = SHIFT_FORMAT;
		ptr = &pdu->format_specific;
		break;
	case AVTP_CVF_FIELD_FORMAT_SUBTYPE:
		mask = MASK_FORMAT_SUBTYPE;
		shift = SHIFT_FORMAT_SUBTYPE;
		ptr = &pdu->format_specific;
		break;
	case AVTP_CVF_FIELD_M:
		mask = MASK_M;
		shift = SHIFT_M;
		ptr = &pdu->packet_info;
		break;
	case AVTP_CVF_FIELD_EVT:
		mask = MASK_EVT;
		shift = SHIFT_EVT;
		ptr = &pdu->packet_info;
		break;
	case AVTP_CVF_FIELD_H264_PTV:
		mask = MASK_PTV;
		shift = SHIFT_PTV;
		ptr = &pdu->packet_info;
		break;
	default:
		return -EINVAL;
	}

	bitmap = ntohl(*ptr);

	BITMAP_SET_VALUE(bitmap, val, mask, shift);

	*ptr = htonl(bitmap);

	return 0;
}

int avtp_cvf_pdu_set(struct avtp_stream_pdu *pdu, enum avtp_cvf_field field,
								uint64_t val)
{
	int res;

	if (!pdu)
		return -EINVAL;

	switch (field) {
	case AVTP_CVF_FIELD_SV:
	case AVTP_CVF_FIELD_MR:
	case AVTP_CVF_FIELD_TV:
	case AVTP_CVF_FIELD_SEQ_NUM:
	case AVTP_CVF_FIELD_TU:
	case AVTP_CVF_FIELD_STREAM_DATA_LEN:
	case AVTP_CVF_FIELD_TIMESTAMP:
	case AVTP_CVF_FIELD_STREAM_ID:
		res = avtp_stream_pdu_set(pdu, (enum avtp_stream_field) field,
									val);
		break;
	case AVTP_CVF_FIELD_FORMAT:
	case AVTP_CVF_FIELD_FORMAT_SUBTYPE:
	case AVTP_CVF_FIELD_M:
	case AVTP_CVF_FIELD_EVT:
	case AVTP_CVF_FIELD_H264_PTV:
		res = set_field_value(pdu, field, val);
		break;
	case AVTP_CVF_FIELD_H264_TIMESTAMP:
	{
		/* This field lives on H.264 header, inside avtp_payload */
		struct avtp_cvf_h264_payload *pay =
			(struct avtp_cvf_h264_payload *)pdu->avtp_payload;
		pay->h264_header = htonl(val);
		res = 0;
		break;
	}
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

int avtp_cvf_pdu_init(struct avtp_stream_pdu *pdu, uint8_t subtype)
{
	int res;

	if (!pdu)
		return -EINVAL;

	if (subtype > AVTP_CVF_FORMAT_SUBTYPE_JPEG2000)
		return -EINVAL;

	memset(pdu, 0, sizeof(struct avtp_stream_pdu));

	res = avtp_pdu_set((struct avtp_common_pdu *) pdu, AVTP_FIELD_SUBTYPE,
							AVTP_SUBTYPE_CVF);
	if (res < 0)
		return res;

	res = avtp_cvf_pdu_set(pdu, AVTP_CVF_FIELD_SV, 1);
	if (res < 0)
		return res;

	res = avtp_cvf_pdu_set(pdu, AVTP_CVF_FIELD_FORMAT, AVTP_CVF_FORMAT_RFC);
	if (res < 0)
		return res;

	res = avtp_cvf_pdu_set(pdu, AVTP_CVF_FIELD_FORMAT_SUBTYPE, subtype);
	if (res < 0)
		return res;

	return 0;
}
