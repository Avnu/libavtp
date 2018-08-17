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

#include <arpa/inet.h>
#include <endian.h>
#include <string.h>

#include "avtp.h"
#include "avtp_crf.h"
#include "util.h"

#define SHIFT_SV			(31 - 8)
#define SHIFT_MR			(31 - 12)
#define SHIFT_FS			(31 - 14)
#define SHIFT_TU			(31 - 15)
#define SHIFT_SEQ_NUM			(31 - 23)
#define SHIFT_PULL			(63 - 2)
#define SHIFT_BASE_FREQ			(63 - 31)
#define SHIFT_CRF_DATA_LEN		(63 - 47)

#define MASK_SV				(BITMASK(1) << SHIFT_SV)
#define MASK_MR				(BITMASK(1) << SHIFT_MR)
#define MASK_FS				(BITMASK(1) << SHIFT_FS)
#define MASK_TU				(BITMASK(1) << SHIFT_TU)
#define MASK_SEQ_NUM			(BITMASK(8) << SHIFT_SEQ_NUM)
#define MASK_TYPE			(BITMASK(8))
#define MASK_PULL			(BITMASK(3) << SHIFT_PULL)
#define MASK_BASE_FREQ			(BITMASK(29) << SHIFT_BASE_FREQ)
#define MASK_CRF_DATA_LEN		(BITMASK(16) << SHIFT_CRF_DATA_LEN)
#define MASK_TIMESTAMP_INTERVAL		(BITMASK(16))

static int get_field_value(const struct avtp_crf_pdu *pdu,
				enum avtp_crf_field field, uint64_t *val)
{
	uint64_t bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_CRF_FIELD_SV:
		mask = MASK_SV;
		shift = SHIFT_SV;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_CRF_FIELD_MR:
		mask = MASK_MR;
		shift = SHIFT_MR;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_CRF_FIELD_FS:
		mask = MASK_FS;
		shift = SHIFT_FS;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_CRF_FIELD_TU:
		mask = MASK_TU;
		shift = SHIFT_TU;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_CRF_FIELD_SEQ_NUM:
		mask = MASK_SEQ_NUM;
		shift = SHIFT_SEQ_NUM;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_CRF_FIELD_TYPE:
		mask = MASK_TYPE;
		shift = 0;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_CRF_FIELD_PULL:
		mask = MASK_PULL;
		shift = SHIFT_PULL;
		bitmap = be64toh(pdu->packet_info);
		break;
	case AVTP_CRF_FIELD_BASE_FREQ:
		mask = MASK_BASE_FREQ;
		shift = SHIFT_BASE_FREQ;
		bitmap = be64toh(pdu->packet_info);
		break;
	case AVTP_CRF_FIELD_CRF_DATA_LEN:
		mask = MASK_CRF_DATA_LEN;
		shift = SHIFT_CRF_DATA_LEN;
		bitmap = be64toh(pdu->packet_info);
		break;
	case AVTP_CRF_FIELD_TIMESTAMP_INTERVAL:
		mask = MASK_TIMESTAMP_INTERVAL;
		shift = 0;
		bitmap = be64toh(pdu->packet_info);
		break;
	default:
		return -EINVAL;
	}

	*val = BITMAP_GET_VALUE(bitmap, mask, shift);

	return 0;
}

int avtp_crf_pdu_get(const struct avtp_crf_pdu *pdu,
				enum avtp_crf_field field, uint64_t *val)
{
	int res;

	if (!pdu || !val)
		return -EINVAL;

	switch (field) {
	case AVTP_CRF_FIELD_SV:
	case AVTP_CRF_FIELD_MR:
	case AVTP_CRF_FIELD_FS:
	case AVTP_CRF_FIELD_TU:
	case AVTP_CRF_FIELD_SEQ_NUM:
	case AVTP_CRF_FIELD_TYPE:
	case AVTP_CRF_FIELD_PULL:
	case AVTP_CRF_FIELD_BASE_FREQ:
	case AVTP_CRF_FIELD_CRF_DATA_LEN:
	case AVTP_CRF_FIELD_TIMESTAMP_INTERVAL:
		res = get_field_value(pdu, field, val);
		break;
	case AVTP_CRF_FIELD_STREAM_ID:
		*val = be64toh(pdu->stream_id);
		res = 0;
		break;
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

static int set_field_value_32(struct avtp_crf_pdu *pdu,
				enum avtp_crf_field field, uint64_t val)
{
	uint32_t bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_CRF_FIELD_SV:
		mask = MASK_SV;
		shift = SHIFT_SV;
		break;
	case AVTP_CRF_FIELD_MR:
		mask = MASK_MR;
		shift = SHIFT_MR;
		break;
	case AVTP_CRF_FIELD_FS:
		mask = MASK_FS;
		shift = SHIFT_FS;
		break;
	case AVTP_CRF_FIELD_TU:
		mask = MASK_TU;
		shift = SHIFT_TU;
		break;
	case AVTP_CRF_FIELD_SEQ_NUM:
		mask = MASK_SEQ_NUM;
		shift = SHIFT_SEQ_NUM;
		break;
	case AVTP_CRF_FIELD_TYPE:
		mask = MASK_TYPE;
		shift = 0;
		break;
	default:
		return -EINVAL;
	}

	bitmap = ntohl(pdu->subtype_data);

	BITMAP_SET_VALUE(bitmap, val, mask, shift);

	pdu->subtype_data = htonl(bitmap);

	return 0;
}

static int set_field_value_64(struct avtp_crf_pdu *pdu,
				enum avtp_crf_field field, uint64_t val)
{
	uint64_t bitmap, mask;
	uint8_t shift;

	switch (field) {
	case AVTP_CRF_FIELD_PULL:
		mask = MASK_PULL;
		shift = SHIFT_PULL;
		break;
	case AVTP_CRF_FIELD_BASE_FREQ:
		mask = MASK_BASE_FREQ;
		shift = SHIFT_BASE_FREQ;
		break;
	case AVTP_CRF_FIELD_CRF_DATA_LEN:
		mask = MASK_CRF_DATA_LEN;
		shift = SHIFT_CRF_DATA_LEN;
		break;
	case AVTP_CRF_FIELD_TIMESTAMP_INTERVAL:
		mask = MASK_TIMESTAMP_INTERVAL;
		shift = 0;
		break;
	default:
		return -EINVAL;
	}

	bitmap = be64toh(pdu->packet_info);

	BITMAP_SET_VALUE(bitmap, val, mask, shift);

	pdu->packet_info = htobe64(bitmap);

	return 0;
}

int avtp_crf_pdu_set(struct avtp_crf_pdu *pdu, enum avtp_crf_field field,
								uint64_t val)
{
	int res;

	if (!pdu)
		return -EINVAL;

	switch (field) {
	case AVTP_CRF_FIELD_SV:
	case AVTP_CRF_FIELD_MR:
	case AVTP_CRF_FIELD_FS:
	case AVTP_CRF_FIELD_TU:
	case AVTP_CRF_FIELD_SEQ_NUM:
	case AVTP_CRF_FIELD_TYPE:
		res = set_field_value_32(pdu, field, val);
		break;
	case AVTP_CRF_FIELD_PULL:
	case AVTP_CRF_FIELD_BASE_FREQ:
	case AVTP_CRF_FIELD_CRF_DATA_LEN:
	case AVTP_CRF_FIELD_TIMESTAMP_INTERVAL:
		res = set_field_value_64(pdu, field, val);
		break;
	case AVTP_CRF_FIELD_STREAM_ID:
		pdu->stream_id = htobe64(val);
		res = 0;
		break;
	default:
		res = -EINVAL;
	}

	return res;
}

int avtp_crf_pdu_init(struct avtp_crf_pdu *pdu)
{
	int res;

	if (!pdu)
		return -EINVAL;

	memset(pdu, 0, sizeof(struct avtp_crf_pdu));

	res = avtp_pdu_set((struct avtp_common_pdu *) pdu, AVTP_FIELD_SUBTYPE,
							AVTP_SUBTYPE_CRF);
	if (res < 0)
		return res;

	res = avtp_crf_pdu_set(pdu, AVTP_CRF_FIELD_SV, 1);
	if (res < 0)
		return res;

	return 0;
}
