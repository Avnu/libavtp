/*
 * Copyright (c) 2017, Intel Corporation
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
#include "avtp_aaf.h"
#include "avtp_stream.h"
#include "util.h"

#define SHIFT_FORMAT			(31 - 7)
#define SHIFT_NSR			(31 - 11)
#define SHIFT_CHAN_PER_FRAME		(31 - 23)
#define SHIFT_SP			(31 - 19)
#define SHIFT_EVT			(31 - 23)

#define MASK_FORMAT			(BITMASK(8) << SHIFT_FORMAT)
#define MASK_NSR			(BITMASK(4) << SHIFT_NSR)
#define MASK_CHAN_PER_FRAME		(BITMASK(10) << SHIFT_CHAN_PER_FRAME)
#define MASK_BIT_DEPTH			(BITMASK(8))
#define MASK_SP				(BITMASK(1) << SHIFT_SP)
#define MASK_EVT			(BITMASK(4) << SHIFT_EVT)

static int get_field_value(const struct avtp_stream_pdu *pdu,
				enum avtp_aaf_field field, uint64_t *val)
{
	uint32_t bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_AAF_FIELD_FORMAT:
		mask = MASK_FORMAT;
		shift = SHIFT_FORMAT;
		bitmap = ntohl(pdu->format_specific);
		break;
	case AVTP_AAF_FIELD_NSR:
		mask = MASK_NSR;
		shift = SHIFT_NSR;
		bitmap = ntohl(pdu->format_specific);
		break;
	case AVTP_AAF_FIELD_CHAN_PER_FRAME:
		mask = MASK_CHAN_PER_FRAME;
		shift = SHIFT_CHAN_PER_FRAME;
		bitmap = ntohl(pdu->format_specific);
		break;
	case AVTP_AAF_FIELD_BIT_DEPTH:
		mask = MASK_BIT_DEPTH;
		shift = 0;
		bitmap = ntohl(pdu->format_specific);
		break;
	case AVTP_AAF_FIELD_SP:
		mask = MASK_SP;
		shift = SHIFT_SP;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_AAF_FIELD_EVT:
		mask = MASK_EVT;
		shift = SHIFT_EVT;
		bitmap = ntohl(pdu->packet_info);
		break;
	default:
		return -EINVAL;
	}

	*val = BITMAP_GET_VALUE(bitmap, mask, shift);

	return 0;
}

int avtp_aaf_pdu_get(const struct avtp_stream_pdu *pdu,
				enum avtp_aaf_field field, uint64_t *val)
{
	int res;

	if (!pdu || !val)
		return -EINVAL;

	switch (field) {
	case AVTP_AAF_FIELD_SV:
	case AVTP_AAF_FIELD_MR:
	case AVTP_AAF_FIELD_TV:
	case AVTP_AAF_FIELD_SEQ_NUM:
	case AVTP_AAF_FIELD_TU:
	case AVTP_AAF_FIELD_STREAM_DATA_LEN:
	case AVTP_AAF_FIELD_TIMESTAMP:
	case AVTP_AAF_FIELD_STREAM_ID:
		res = avtp_stream_pdu_get(pdu, (enum avtp_stream_field) field,
									val);
		break;
	case AVTP_AAF_FIELD_FORMAT:
	case AVTP_AAF_FIELD_NSR:
	case AVTP_AAF_FIELD_CHAN_PER_FRAME:
	case AVTP_AAF_FIELD_BIT_DEPTH:
	case AVTP_AAF_FIELD_SP:
	case AVTP_AAF_FIELD_EVT:
		res = get_field_value(pdu, field, val);
		break;
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

static int set_field_value(struct avtp_stream_pdu *pdu,
				enum avtp_aaf_field field, uint32_t val)
{
	uint32_t bitmap, mask;
	uint8_t shift;
	void *ptr;

	switch (field) {
	case AVTP_AAF_FIELD_FORMAT:
		mask = MASK_FORMAT;
		shift = SHIFT_FORMAT;
		ptr = &pdu->format_specific;
		break;
	case AVTP_AAF_FIELD_NSR:
		mask = MASK_NSR;
		shift = SHIFT_NSR;
		ptr = &pdu->format_specific;
		break;
	case AVTP_AAF_FIELD_CHAN_PER_FRAME:
		mask = MASK_CHAN_PER_FRAME;
		shift = SHIFT_CHAN_PER_FRAME;
		ptr = &pdu->format_specific;
		break;
	case AVTP_AAF_FIELD_BIT_DEPTH:
		mask = MASK_BIT_DEPTH;
		shift = 0;
		ptr = &pdu->format_specific;
		break;
	case AVTP_AAF_FIELD_SP:
		mask = MASK_SP;
		shift = SHIFT_SP;
		ptr = &pdu->packet_info;
		break;
	case AVTP_AAF_FIELD_EVT:
		mask = MASK_EVT;
		shift = SHIFT_EVT;
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

int avtp_aaf_pdu_set(struct avtp_stream_pdu *pdu, enum avtp_aaf_field field,
								uint64_t val)
{
	int res;

	if (!pdu)
		return -EINVAL;

	switch (field) {
	case AVTP_AAF_FIELD_SV:
	case AVTP_AAF_FIELD_MR:
	case AVTP_AAF_FIELD_TV:
	case AVTP_AAF_FIELD_SEQ_NUM:
	case AVTP_AAF_FIELD_TU:
	case AVTP_AAF_FIELD_STREAM_DATA_LEN:
	case AVTP_AAF_FIELD_TIMESTAMP:
	case AVTP_AAF_FIELD_STREAM_ID:
		res = avtp_stream_pdu_set(pdu, (enum avtp_stream_field) field,
									val);
		break;
	case AVTP_AAF_FIELD_FORMAT:
	case AVTP_AAF_FIELD_NSR:
	case AVTP_AAF_FIELD_CHAN_PER_FRAME:
	case AVTP_AAF_FIELD_BIT_DEPTH:
	case AVTP_AAF_FIELD_SP:
	case AVTP_AAF_FIELD_EVT:
		res = set_field_value(pdu, field, val);
		break;
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

int avtp_aaf_pdu_init(struct avtp_stream_pdu *pdu)
{
	int res;

	if (!pdu)
		return -EINVAL;

	memset(pdu, 0, sizeof(struct avtp_stream_pdu));

	res = avtp_pdu_set((struct avtp_common_pdu *) pdu, AVTP_FIELD_SUBTYPE,
							AVTP_SUBTYPE_AAF);
	if (res < 0)
		return res;

	res = avtp_aaf_pdu_set(pdu, AVTP_AAF_FIELD_SV, 1);
	if (res < 0)
		return res;

	return 0;
};
