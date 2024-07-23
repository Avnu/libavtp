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
#include <endian.h>
#include <stddef.h>

#include "avtp.h"
#include "avtp_stream.h"
#include "util.h"

#define SHIFT_SV			(31 - 8)
#define SHIFT_MR			(31 - 12)
#define SHIFT_TV			(31 - 15)
#define SHIFT_SEQ_NUM			(31 - 23)
#define SHIFT_STREAM_DATA_LEN		(31 - 15)

#define MASK_SV				(BITMASK(1) << SHIFT_SV)
#define MASK_MR				(BITMASK(1) << SHIFT_MR)
#define MASK_TV				(BITMASK(1) << SHIFT_TV)
#define MASK_SEQ_NUM			(BITMASK(8) << SHIFT_SEQ_NUM)
#define MASK_TU				(BITMASK(1))
#define MASK_STREAM_DATA_LEN		(BITMASK(16) << SHIFT_STREAM_DATA_LEN)

static int get_field_value(const struct avtp_stream_pdu *pdu,
				enum avtp_stream_field field, uint64_t *val)
{
	uint32_t bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_STREAM_FIELD_SV:
		mask = MASK_SV;
		shift = SHIFT_SV;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_STREAM_FIELD_MR:
		mask = MASK_MR;
		shift = SHIFT_MR;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_STREAM_FIELD_TV:
		mask = MASK_TV;
		shift = SHIFT_TV;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_STREAM_FIELD_SEQ_NUM:
		mask = MASK_SEQ_NUM;
		shift = SHIFT_SEQ_NUM;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_STREAM_FIELD_TU:
		mask = MASK_TU;
		shift = 0;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_STREAM_FIELD_STREAM_DATA_LEN:
		mask = MASK_STREAM_DATA_LEN;
		shift = SHIFT_STREAM_DATA_LEN;
		bitmap = ntohl(pdu->packet_info);
		break;
	default:
		return -EINVAL;
	}

	*val = BITMAP_GET_VALUE(bitmap, mask, shift);

	return 0;
}

int avtp_stream_pdu_get(const struct avtp_stream_pdu *pdu,
				enum avtp_stream_field field, uint64_t *val)
{
	int res;

	if (!pdu || !val)
		return -EINVAL;

	switch (field) {
	case AVTP_STREAM_FIELD_SV:
	case AVTP_STREAM_FIELD_MR:
	case AVTP_STREAM_FIELD_TV:
	case AVTP_STREAM_FIELD_SEQ_NUM:
	case AVTP_STREAM_FIELD_TU:
	case AVTP_STREAM_FIELD_STREAM_DATA_LEN:
		res = get_field_value(pdu, field, val);
		break;
	case AVTP_STREAM_FIELD_TIMESTAMP:
		*val = ntohl(pdu->avtp_time);
		res = 0;
		break;
	case AVTP_STREAM_FIELD_STREAM_ID:
		*val = be64toh(pdu->stream_id);
		res = 0;
		break;
	default:
		return -EINVAL;
	}

	return res;
}

static int set_field_value(struct avtp_stream_pdu *pdu,
				enum avtp_stream_field field, uint64_t val)
{
	uint32_t bitmap, mask;
	uint8_t shift;
	void *ptr;

	switch (field) {
	case AVTP_STREAM_FIELD_SV:
		mask = MASK_SV;
		shift = SHIFT_SV;
		ptr = &pdu->subtype_data;
		break;
	case AVTP_STREAM_FIELD_MR:
		mask = MASK_MR;
		shift = SHIFT_MR;
		ptr = &pdu->subtype_data;
		break;
	case AVTP_STREAM_FIELD_TV:
		mask = MASK_TV;
		shift = SHIFT_TV;
		ptr = &pdu->subtype_data;
		break;
	case AVTP_STREAM_FIELD_SEQ_NUM:
		mask = MASK_SEQ_NUM;
		shift = SHIFT_SEQ_NUM;
		ptr = &pdu->subtype_data;
		break;
	case AVTP_STREAM_FIELD_TU:
		mask = MASK_TU;
		shift = 0;
		ptr = &pdu->subtype_data;
		break;
	case AVTP_STREAM_FIELD_STREAM_DATA_LEN:
		mask = MASK_STREAM_DATA_LEN;
		shift = SHIFT_STREAM_DATA_LEN;
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

int avtp_stream_pdu_set(struct avtp_stream_pdu *pdu,
				enum avtp_stream_field field, uint64_t value)
{
	int res;

	if (!pdu)
		return -EINVAL;

	switch (field) {
	case AVTP_STREAM_FIELD_SV:
	case AVTP_STREAM_FIELD_MR:
	case AVTP_STREAM_FIELD_TV:
	case AVTP_STREAM_FIELD_SEQ_NUM:
	case AVTP_STREAM_FIELD_TU:
	case AVTP_STREAM_FIELD_STREAM_DATA_LEN:
		res = set_field_value(pdu, field, value);
		break;
	case AVTP_STREAM_FIELD_TIMESTAMP:
		pdu->avtp_time = htonl(value);
		res = 0;
		break;
	case AVTP_STREAM_FIELD_STREAM_ID:
		pdu->stream_id = htobe64(value);
		res = 0;
		break;
	default:
		return -EINVAL;
	}

	return res;
}
