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
#include "avtp_rvf.h"
#include "avtp_stream.h"
#include "util.h"

#define SHIFT_ACTIVE_PIXELS    (31 - 15)
#define SHIFT_TOTAL_LINES      (31 - 31)
#define SHIFT_AP	       (31 - 16)
#define SHIFT_F		       (31 - 18)
#define SHIFT_EF	       (31 - 19)
#define SHIFT_EVT	       (31 - 23)
#define SHIFT_PD	       (31 - 24)
#define SHIFT_I		       (31 - 25)
#define SHIFT_RAW_PIXEL_DEPTH  (63 - 11)
#define SHIFT_RAW_PIXEL_FORMAT (63 - 15)
#define SHIFT_RAW_FRAME_RATE   (63 - 23)
#define SHIFT_RAW_COLORSPACE   (63 - 27)
#define SHIFT_RAW_NUM_LINES    (63 - 31)
#define SHIFT_RAW_I_SEQ_NUM    (63 - 47)
#define SHIFT_RAW_LINE_NUMBER  (63 - 63)

#define MASK_ACTIVE_PIXELS    (BITMASK(16) << SHIFT_ACTIVE_PIXELS)
#define MASK_TOTAL_LINES      (BITMASK(16) << SHIFT_TOTAL_LINES)
#define MASK_AP		      (BITMASK(1) << SHIFT_AP)
#define MASK_F		      (BITMASK(1) << SHIFT_F)
#define MASK_EF		      (BITMASK(1) << SHIFT_EF)
#define MASK_EVT	      (BITMASK(4) << SHIFT_EVT)
#define MASK_PD		      (BITMASK(1) << SHIFT_PD)
#define MASK_I		      (BITMASK(1) << SHIFT_I)
#define MASK_RAW_PIXEL_DEPTH  (BITMASK(4) << SHIFT_RAW_PIXEL_DEPTH)
#define MASK_RAW_PIXEL_FORMAT (BITMASK(4) << SHIFT_RAW_PIXEL_FORMAT)
#define MASK_RAW_FRAME_RATE   (BITMASK(8) << SHIFT_RAW_FRAME_RATE)
#define MASK_RAW_COLORSPACE   (BITMASK(4) << SHIFT_RAW_COLORSPACE)
#define MASK_RAW_NUM_LINES    (BITMASK(4) << SHIFT_RAW_NUM_LINES)
#define MASK_RAW_I_SEQ_NUM    (BITMASK(8) << SHIFT_RAW_I_SEQ_NUM)
#define MASK_RAW_LINE_NUMBER  (BITMASK(16) << SHIFT_RAW_LINE_NUMBER)

static int get_field_value(const struct avtp_stream_pdu *pdu,
			   enum avtp_rvf_field field, uint64_t *val)
{
	uint32_t bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_RVF_FIELD_ACTIVE_PIXELS:
		mask = MASK_ACTIVE_PIXELS;
		shift = SHIFT_ACTIVE_PIXELS;
		bitmap = ntohl(pdu->format_specific);
		break;
	case AVTP_RVF_FIELD_TOTAL_LINES:
		mask = MASK_TOTAL_LINES;
		shift = SHIFT_TOTAL_LINES;
		bitmap = ntohl(pdu->format_specific);
		break;
	case AVTP_RVF_FIELD_AP:
		mask = MASK_AP;
		shift = SHIFT_AP;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_RVF_FIELD_F:
		mask = MASK_F;
		shift = SHIFT_F;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_RVF_FIELD_EF:
		mask = MASK_EF;
		shift = SHIFT_EF;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_RVF_FIELD_EVT:
		mask = MASK_EVT;
		shift = SHIFT_EVT;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_RVF_FIELD_PD:
		mask = MASK_PD;
		shift = SHIFT_PD;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_RVF_FIELD_I:
		mask = MASK_I;
		shift = SHIFT_I;
		bitmap = ntohl(pdu->packet_info);
		break;
	default:
		return -EINVAL;
	}

	*val = BITMAP_GET_VALUE(bitmap, mask, shift);

	return 0;
}

static int get_raw_field_value(const uint64_t bitmap, enum avtp_rvf_field field,
			       uint64_t *val)
{
	uint64_t mask;
	uint8_t shift;

	switch (field) {
	case AVTP_RVF_FIELD_RAW_PIXEL_DEPTH:
		mask = MASK_RAW_PIXEL_DEPTH;
		shift = SHIFT_RAW_PIXEL_DEPTH;
		break;
	case AVTP_RVF_FIELD_RAW_PIXEL_FORMAT:
		mask = MASK_RAW_PIXEL_FORMAT;
		shift = SHIFT_RAW_PIXEL_FORMAT;
		break;
	case AVTP_RVF_FIELD_RAW_FRAME_RATE:
		mask = MASK_RAW_FRAME_RATE;
		shift = SHIFT_RAW_FRAME_RATE;
		break;
	case AVTP_RVF_FIELD_RAW_COLORSPACE:
		mask = MASK_RAW_COLORSPACE;
		shift = SHIFT_RAW_COLORSPACE;
		break;
	case AVTP_RVF_FIELD_RAW_NUM_LINES:
		mask = MASK_RAW_NUM_LINES;
		shift = SHIFT_RAW_NUM_LINES;
		break;
	case AVTP_RVF_FIELD_RAW_I_SEQ_NUM:
		mask = MASK_RAW_I_SEQ_NUM;
		shift = SHIFT_RAW_I_SEQ_NUM;
		break;
	case AVTP_RVF_FIELD_RAW_LINE_NUMBER:
		mask = MASK_RAW_LINE_NUMBER;
		shift = SHIFT_RAW_LINE_NUMBER;
		break;
	default:
		return -EINVAL;
	}

	*val = BITMAP_GET_VALUE(bitmap, mask, shift);

	return 0;
}

int avtp_rvf_pdu_get(const struct avtp_stream_pdu *pdu,
		     enum avtp_rvf_field field, uint64_t *val)
{
	int res;

	if (!pdu || !val)
		return -EINVAL;

	switch (field) {
	case AVTP_RVF_FIELD_SV:
	case AVTP_RVF_FIELD_MR:
	case AVTP_RVF_FIELD_TV:
	case AVTP_RVF_FIELD_SEQ_NUM:
	case AVTP_RVF_FIELD_TU:
	case AVTP_RVF_FIELD_STREAM_DATA_LEN:
	case AVTP_RVF_FIELD_TIMESTAMP:
	case AVTP_RVF_FIELD_STREAM_ID:
		res = avtp_stream_pdu_get(pdu, (enum avtp_stream_field)field,
					  val);
		break;
	case AVTP_RVF_FIELD_ACTIVE_PIXELS:
	case AVTP_RVF_FIELD_TOTAL_LINES:
	case AVTP_RVF_FIELD_AP:
	case AVTP_RVF_FIELD_F:
	case AVTP_RVF_FIELD_EF:
	case AVTP_RVF_FIELD_EVT:
	case AVTP_RVF_FIELD_PD:
	case AVTP_RVF_FIELD_I:
		res = get_field_value(pdu, field, val);
		break;
	case AVTP_RVF_FIELD_RAW_PIXEL_DEPTH:
	case AVTP_RVF_FIELD_RAW_PIXEL_FORMAT:
	case AVTP_RVF_FIELD_RAW_FRAME_RATE:
	case AVTP_RVF_FIELD_RAW_COLORSPACE:
	case AVTP_RVF_FIELD_RAW_NUM_LINES:
	case AVTP_RVF_FIELD_RAW_I_SEQ_NUM:
	case AVTP_RVF_FIELD_RAW_LINE_NUMBER: {
		/* These fields lives on RAW header, inside avtp_payload */
		struct avtp_rvf_payload *pay =
			(struct avtp_rvf_payload *)pdu->avtp_payload;
		res = get_raw_field_value(be64toh(pay->raw_header), field, val);
		break;
	}
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

static int set_field_value(struct avtp_stream_pdu *pdu,
			   enum avtp_rvf_field field, uint32_t val)
{
	uint32_t bitmap, mask;
	uint8_t shift;
	void *ptr;

	switch (field) {
	case AVTP_RVF_FIELD_ACTIVE_PIXELS:
		mask = MASK_ACTIVE_PIXELS;
		shift = SHIFT_ACTIVE_PIXELS;
		ptr = &pdu->format_specific;
		break;
	case AVTP_RVF_FIELD_TOTAL_LINES:
		mask = MASK_TOTAL_LINES;
		shift = SHIFT_TOTAL_LINES;
		ptr = &pdu->format_specific;
		break;
	case AVTP_RVF_FIELD_AP:
		mask = MASK_AP;
		shift = SHIFT_AP;
		ptr = &pdu->packet_info;
		break;
	case AVTP_RVF_FIELD_F:
		mask = MASK_F;
		shift = SHIFT_F;
		ptr = &pdu->packet_info;
		break;
	case AVTP_RVF_FIELD_EF:
		mask = MASK_EF;
		shift = SHIFT_EF;
		ptr = &pdu->packet_info;
		break;
	case AVTP_RVF_FIELD_EVT:
		mask = MASK_EVT;
		shift = SHIFT_EVT;
		ptr = &pdu->packet_info;
		break;
	case AVTP_RVF_FIELD_PD:
		mask = MASK_PD;
		shift = SHIFT_PD;
		ptr = &pdu->packet_info;
		break;
	case AVTP_RVF_FIELD_I:
		mask = MASK_I;
		shift = SHIFT_I;
		ptr = &pdu->packet_info;
		break;
	default:
		return -EINVAL;
	}

	bitmap = get_unaligned_be32(ptr);

	BITMAP_SET_VALUE(bitmap, val, mask, shift);

	put_unaligned_be32(bitmap, ptr);

	return 0;
}

static int set_raw_field_value(struct avtp_rvf_payload *pay,
			       enum avtp_rvf_field field, uint64_t val)
{
	uint64_t bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_RVF_FIELD_RAW_PIXEL_DEPTH:
		mask = MASK_RAW_PIXEL_DEPTH;
		shift = SHIFT_RAW_PIXEL_DEPTH;
		break;
	case AVTP_RVF_FIELD_RAW_PIXEL_FORMAT:
		mask = MASK_RAW_PIXEL_FORMAT;
		shift = SHIFT_RAW_PIXEL_FORMAT;
		break;
	case AVTP_RVF_FIELD_RAW_FRAME_RATE:
		mask = MASK_RAW_FRAME_RATE;
		shift = SHIFT_RAW_FRAME_RATE;
		break;
	case AVTP_RVF_FIELD_RAW_COLORSPACE:
		mask = MASK_RAW_COLORSPACE;
		shift = SHIFT_RAW_COLORSPACE;
		break;
	case AVTP_RVF_FIELD_RAW_NUM_LINES:
		mask = MASK_RAW_NUM_LINES;
		shift = SHIFT_RAW_NUM_LINES;
		break;
	case AVTP_RVF_FIELD_RAW_I_SEQ_NUM:
		mask = MASK_RAW_I_SEQ_NUM;
		shift = SHIFT_RAW_I_SEQ_NUM;
		break;
	case AVTP_RVF_FIELD_RAW_LINE_NUMBER:
		mask = MASK_RAW_LINE_NUMBER;
		shift = SHIFT_RAW_LINE_NUMBER;
		break;
	default:
		return -EINVAL;
	}

	bitmap = be64toh(pay->raw_header);

	BITMAP_SET_VALUE(bitmap, val, mask, shift);

	pay->raw_header = htobe64(bitmap);

	return 0;
}

int avtp_rvf_pdu_set(struct avtp_stream_pdu *pdu, enum avtp_rvf_field field,
		     uint64_t val)
{
	int res;

	if (!pdu)
		return -EINVAL;

	switch (field) {
	case AVTP_RVF_FIELD_SV:
	case AVTP_RVF_FIELD_MR:
	case AVTP_RVF_FIELD_TV:
	case AVTP_RVF_FIELD_SEQ_NUM:
	case AVTP_RVF_FIELD_TU:
	case AVTP_RVF_FIELD_STREAM_DATA_LEN:
	case AVTP_RVF_FIELD_TIMESTAMP:
	case AVTP_RVF_FIELD_STREAM_ID:
		res = avtp_stream_pdu_set(pdu, (enum avtp_stream_field)field,
					  val);
		break;
	case AVTP_RVF_FIELD_ACTIVE_PIXELS:
	case AVTP_RVF_FIELD_TOTAL_LINES:
	case AVTP_RVF_FIELD_AP:
	case AVTP_RVF_FIELD_F:
	case AVTP_RVF_FIELD_EF:
	case AVTP_RVF_FIELD_EVT:
	case AVTP_RVF_FIELD_PD:
	case AVTP_RVF_FIELD_I:
		res = set_field_value(pdu, field, val);
		break;
	case AVTP_RVF_FIELD_RAW_PIXEL_DEPTH:
	case AVTP_RVF_FIELD_RAW_PIXEL_FORMAT:
	case AVTP_RVF_FIELD_RAW_FRAME_RATE:
	case AVTP_RVF_FIELD_RAW_COLORSPACE:
	case AVTP_RVF_FIELD_RAW_NUM_LINES:
	case AVTP_RVF_FIELD_RAW_I_SEQ_NUM:
	case AVTP_RVF_FIELD_RAW_LINE_NUMBER: {
		/* These fields lives on RAW header, inside avtp_payload */
		res = set_raw_field_value(
			(struct avtp_rvf_payload *)pdu->avtp_payload, field,
			val);
		break;
	}
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

int avtp_rvf_pdu_init(struct avtp_stream_pdu *pdu)
{
	int res;

	if (!pdu)
		return -EINVAL;

	memset(pdu, 0, sizeof(struct avtp_stream_pdu));

	res = avtp_pdu_set((struct avtp_common_pdu *)pdu, AVTP_FIELD_SUBTYPE,
			   AVTP_SUBTYPE_RVF);
	if (res < 0)
		return res;

	res = avtp_rvf_pdu_set(pdu, AVTP_RVF_FIELD_SV, 1);
	if (res < 0)
		return res;

	return 0;
}
