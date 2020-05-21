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
#include <string.h>

#include "avtp.h"
#include "avtp_ieciidc.h"
#include "avtp_stream.h"
#include "util.h"

#define SHIFT_GV			(31 - 14)
#define SHIFT_TAG			(31 - 17)
#define SHIFT_CHANNEL			(31 - 23)
#define SHIFT_TCODE			(31 - 27)
#define SHIFT_QI_1			(31 - 1)
#define SHIFT_QI_2			(31 - 1)
#define SHIFT_SID			(31 - 7)
#define SHIFT_DBS			(31 - 15)
#define SHIFT_FN			(31 - 17)
#define SHIFT_QPC			(31 - 20)
#define SHIFT_SPH			(31 - 21)
#define SHIFT_SPH			(31 - 21)
#define SHIFT_FMT			(31 - 7)
#define SHIFT_TSF			(31 - 8)
#define SHIFT_EVT			(31 - 11)
#define SHIFT_SFC			(31 - 15)
#define SHIFT_N				(31 - 12)
#define SHIFT_NO_DATA			(31 - 15)
#define SHIFT_ND			(31 - 8)

#define MASK_GV				(BITMASK(1) << SHIFT_GV)
#define MASK_TAG			(BITMASK(2) << SHIFT_TAG)
#define MASK_CHANNEL			(BITMASK(6) << SHIFT_CHANNEL)
#define MASK_TCODE			(BITMASK(4) << SHIFT_TCODE)
#define MASK_SY				(BITMASK(4))
#define MASK_QI_1			(BITMASK(2) << SHIFT_QI_1)
#define MASK_QI_2			(BITMASK(2) << SHIFT_QI_2)
#define MASK_SID			(BITMASK(6) << SHIFT_SID)
#define MASK_DBS			(BITMASK(8) << SHIFT_DBS)
#define MASK_FN				(BITMASK(2) << SHIFT_FN)
#define MASK_QPC			(BITMASK(3) << SHIFT_QPC)
#define MASK_SPH			(BITMASK(1) << SHIFT_SPH)
#define MASK_DBC			(BITMASK(8))
#define MASK_FMT			(BITMASK(6) << SHIFT_FMT)
#define MASK_SYT			(BITMASK(16))
#define MASK_TSF			(BITMASK(1) << SHIFT_TSF)
#define MASK_EVT			(BITMASK(2) << SHIFT_EVT)
#define MASK_SFC			(BITMASK(3) << SHIFT_SFC)
#define MASK_N				(BITMASK(1) << SHIFT_N)
#define MASK_NO_DATA			(BITMASK(8) << SHIFT_NO_DATA)
#define MASK_ND				(BITMASK(1) << SHIFT_ND)

static int get_field_value(const struct avtp_stream_pdu *pdu,
				enum avtp_ieciidc_field field, uint64_t *val)
{
	uint32_t bitmap, mask;
	uint8_t shift;

	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) pdu->avtp_payload;

	switch (field) {
	case AVTP_IECIIDC_FIELD_GV:
		mask = MASK_GV;
		shift = SHIFT_GV;
		bitmap = ntohl(pdu->subtype_data);
		break;
	case AVTP_IECIIDC_FIELD_TAG:
		mask = MASK_TAG;
		shift = SHIFT_TAG;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_IECIIDC_FIELD_CHANNEL:
		mask = MASK_CHANNEL;
		shift = SHIFT_CHANNEL;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_IECIIDC_FIELD_TCODE:
		mask = MASK_TCODE;
		shift = SHIFT_TCODE;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_IECIIDC_FIELD_SY:
		mask = MASK_SY;
		shift = 0;
		bitmap = ntohl(pdu->packet_info);
		break;
	case AVTP_IECIIDC_FIELD_CIP_QI_1:
		mask = MASK_QI_1;
		shift = SHIFT_QI_1;
		bitmap = ntohl(pay->cip_1);
		break;
	case AVTP_IECIIDC_FIELD_CIP_QI_2:
		mask = MASK_QI_2;
		shift = SHIFT_QI_2;
		bitmap = ntohl(pay->cip_2);
		break;
	case AVTP_IECIIDC_FIELD_CIP_SID:
		mask = MASK_SID;
		shift = SHIFT_SID;
		bitmap = ntohl(pay->cip_1);
		break;
	case AVTP_IECIIDC_FIELD_CIP_DBS:
		mask = MASK_DBS;
		shift = SHIFT_DBS;
		bitmap = ntohl(pay->cip_1);
		break;
	case AVTP_IECIIDC_FIELD_CIP_FN:
		mask = MASK_FN;
		shift = SHIFT_FN;
		bitmap = ntohl(pay->cip_1);
		break;
	case AVTP_IECIIDC_FIELD_CIP_QPC:
		mask = MASK_QPC;
		shift = SHIFT_QPC;
		bitmap = ntohl(pay->cip_1);
		break;
	case AVTP_IECIIDC_FIELD_CIP_SPH:
		mask = MASK_SPH;
		shift = SHIFT_SPH;
		bitmap = ntohl(pay->cip_1);
		break;
	case AVTP_IECIIDC_FIELD_CIP_DBC:
		mask = MASK_DBC;
		shift = 0;
		bitmap = ntohl(pay->cip_1);
		break;
	case AVTP_IECIIDC_FIELD_CIP_FMT:
		mask = MASK_FMT;
		shift = SHIFT_FMT;
		bitmap = ntohl(pay->cip_2);
		break;
	case AVTP_IECIIDC_FIELD_CIP_TSF:
		mask = MASK_TSF;
		shift = SHIFT_TSF;
		bitmap = ntohl(pay->cip_2);
		break;
	case AVTP_IECIIDC_FIELD_CIP_EVT:
		mask = MASK_EVT;
		shift = SHIFT_EVT;
		bitmap = ntohl(pay->cip_2);
		break;
	case AVTP_IECIIDC_FIELD_CIP_SFC:
		mask = MASK_SFC;
		shift = SHIFT_SFC;
		bitmap = ntohl(pay->cip_2);
		break;
	case AVTP_IECIIDC_FIELD_CIP_N:
		mask = MASK_N;
		shift = SHIFT_N;
		bitmap = ntohl(pay->cip_2);
		break;
	case AVTP_IECIIDC_FIELD_CIP_ND:
		mask = MASK_ND;
		shift = SHIFT_ND;
		bitmap = ntohl(pay->cip_2);
		break;
	case AVTP_IECIIDC_FIELD_CIP_NO_DATA:
		mask = MASK_NO_DATA;
		shift = SHIFT_NO_DATA;
		bitmap = ntohl(pay->cip_2);
		break;
	case AVTP_IECIIDC_FIELD_CIP_SYT:
		mask = MASK_SYT;
		shift = 0;
		bitmap = ntohl(pay->cip_2);
		break;
	default:
		return -EINVAL;
	}

	*val = BITMAP_GET_VALUE(bitmap, mask, shift);

	return 0;
}

int avtp_ieciidc_pdu_get(const struct avtp_stream_pdu *pdu,
				enum avtp_ieciidc_field field, uint64_t *val)
{
	int res;

	if (!pdu || !val)
		return -EINVAL;

	switch (field) {
	case AVTP_IECIIDC_FIELD_SV:
	case AVTP_IECIIDC_FIELD_MR:
	case AVTP_IECIIDC_FIELD_TV:
	case AVTP_IECIIDC_FIELD_SEQ_NUM:
	case AVTP_IECIIDC_FIELD_TU:
	case AVTP_IECIIDC_FIELD_STREAM_DATA_LEN:
	case AVTP_IECIIDC_FIELD_TIMESTAMP:
	case AVTP_IECIIDC_FIELD_STREAM_ID:
		res = avtp_stream_pdu_get(pdu, (enum avtp_stream_field) field,
									val);
		break;
	case AVTP_IECIIDC_FIELD_GV:
	case AVTP_IECIIDC_FIELD_TAG:
	case AVTP_IECIIDC_FIELD_CHANNEL:
	case AVTP_IECIIDC_FIELD_TCODE:
	case AVTP_IECIIDC_FIELD_SY:
	case AVTP_IECIIDC_FIELD_CIP_QI_1:
	case AVTP_IECIIDC_FIELD_CIP_QI_2:
	case AVTP_IECIIDC_FIELD_CIP_SID:
	case AVTP_IECIIDC_FIELD_CIP_DBS:
	case AVTP_IECIIDC_FIELD_CIP_FN:
	case AVTP_IECIIDC_FIELD_CIP_QPC:
	case AVTP_IECIIDC_FIELD_CIP_SPH:
	case AVTP_IECIIDC_FIELD_CIP_DBC:
	case AVTP_IECIIDC_FIELD_CIP_FMT:
	case AVTP_IECIIDC_FIELD_CIP_TSF:
	case AVTP_IECIIDC_FIELD_CIP_EVT:
	case AVTP_IECIIDC_FIELD_CIP_SFC:
	case AVTP_IECIIDC_FIELD_CIP_N:
	case AVTP_IECIIDC_FIELD_CIP_ND:
	case AVTP_IECIIDC_FIELD_CIP_NO_DATA:
	case AVTP_IECIIDC_FIELD_CIP_SYT:
		res = get_field_value(pdu, field, val);
		break;
	case AVTP_IECIIDC_FIELD_GATEWAY_INFO:
		*val = ntohl(pdu->format_specific);
		res = 0;
		break;
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

static int set_field_value(struct avtp_stream_pdu *pdu,
			enum avtp_ieciidc_field field, uint64_t value)
{
	uint32_t bitmap, mask;
	uint8_t shift;
	void *ptr;

	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) pdu->avtp_payload;

	switch (field) {
	case AVTP_IECIIDC_FIELD_GV:
		mask = MASK_GV;
		shift = SHIFT_GV;
		ptr = &pdu->subtype_data;
		break;
	case AVTP_IECIIDC_FIELD_TAG:
		mask = MASK_TAG;
		shift = SHIFT_TAG;
		ptr = &pdu->packet_info;
		break;
	case AVTP_IECIIDC_FIELD_CHANNEL:
		mask = MASK_CHANNEL;
		shift = SHIFT_CHANNEL;
		ptr = &pdu->packet_info;
		break;
	case AVTP_IECIIDC_FIELD_TCODE:
		mask = MASK_TCODE;
		shift = SHIFT_TCODE;
		ptr = &pdu->packet_info;
		break;
	case AVTP_IECIIDC_FIELD_SY:
		mask = MASK_SY;
		shift = 0;
		ptr = &pdu->packet_info;
		break;
	case AVTP_IECIIDC_FIELD_CIP_QI_1:
		mask = MASK_QI_1;
		shift = SHIFT_QI_1;
		ptr = &pay->cip_1;
		break;
	case AVTP_IECIIDC_FIELD_CIP_QI_2:
		mask = MASK_QI_2;
		shift = SHIFT_QI_2;
		ptr = &pay->cip_2;
		break;
	case AVTP_IECIIDC_FIELD_CIP_SID:
		mask = MASK_SID;
		shift = SHIFT_SID;
		ptr = &pay->cip_1;
		break;
	case AVTP_IECIIDC_FIELD_CIP_DBS:
		mask = MASK_DBS;
		shift = SHIFT_DBS;
		ptr = &pay->cip_1;
		break;
	case AVTP_IECIIDC_FIELD_CIP_FN:
		mask = MASK_FN;
		shift = SHIFT_FN;
		ptr = &pay->cip_1;
		break;
	case AVTP_IECIIDC_FIELD_CIP_QPC:
		mask = MASK_QPC;
		shift = SHIFT_QPC;
		ptr = &pay->cip_1;
		break;
	case AVTP_IECIIDC_FIELD_CIP_SPH:
		mask = MASK_SPH;
		shift = SHIFT_SPH;
		ptr = &pay->cip_1;
		break;
	case AVTP_IECIIDC_FIELD_CIP_DBC:
		mask = MASK_DBC;
		shift = 0;
		ptr = &pay->cip_1;
		break;
	case AVTP_IECIIDC_FIELD_CIP_FMT:
		mask = MASK_FMT;
		shift = SHIFT_FMT;
		ptr = &pay->cip_2;
		break;
	case AVTP_IECIIDC_FIELD_CIP_TSF:
		mask = MASK_TSF;
		shift = SHIFT_TSF;
		ptr = &pay->cip_2;
		break;
	case AVTP_IECIIDC_FIELD_CIP_EVT:
		mask = MASK_EVT;
		shift = SHIFT_EVT;
		ptr = &pay->cip_2;
		break;
	case AVTP_IECIIDC_FIELD_CIP_SFC:
		mask = MASK_SFC;
		shift = SHIFT_SFC;
		ptr = &pay->cip_2;
		break;
	case AVTP_IECIIDC_FIELD_CIP_N:
		mask = MASK_N;
		shift = SHIFT_N;
		ptr = &pay->cip_2;
		break;
	case AVTP_IECIIDC_FIELD_CIP_ND:
		mask = MASK_ND;
		shift = SHIFT_ND;
		ptr = &pay->cip_2;
		break;
	case AVTP_IECIIDC_FIELD_CIP_NO_DATA:
		mask = MASK_NO_DATA;
		shift = SHIFT_NO_DATA;
		ptr = &pay->cip_2;
		break;
	case AVTP_IECIIDC_FIELD_CIP_SYT:
		mask = MASK_SYT;
		shift = 0;
		ptr = &pay->cip_2;
		break;
	default:
		return -EINVAL;
	}

	bitmap = get_unaligned_be32(ptr);

	BITMAP_SET_VALUE(bitmap, value, mask, shift);

	put_unaligned_be32(bitmap, ptr);

	return 0;
}

int avtp_ieciidc_pdu_set(struct avtp_stream_pdu *pdu,
			enum avtp_ieciidc_field field, uint64_t value)
{
	int res;

	if (!pdu)
		return -EINVAL;

	switch (field) {
	case AVTP_IECIIDC_FIELD_SV:
	case AVTP_IECIIDC_FIELD_MR:
	case AVTP_IECIIDC_FIELD_TV:
	case AVTP_IECIIDC_FIELD_SEQ_NUM:
	case AVTP_IECIIDC_FIELD_TU:
	case AVTP_IECIIDC_FIELD_STREAM_DATA_LEN:
	case AVTP_IECIIDC_FIELD_TIMESTAMP:
	case AVTP_IECIIDC_FIELD_STREAM_ID:
		res = avtp_stream_pdu_set(pdu, (enum avtp_stream_field) field,
									value);
		break;
	case AVTP_IECIIDC_FIELD_GV:
	case AVTP_IECIIDC_FIELD_TAG:
	case AVTP_IECIIDC_FIELD_CHANNEL:
	case AVTP_IECIIDC_FIELD_TCODE:
	case AVTP_IECIIDC_FIELD_SY:
	case AVTP_IECIIDC_FIELD_CIP_QI_1:
	case AVTP_IECIIDC_FIELD_CIP_QI_2:
	case AVTP_IECIIDC_FIELD_CIP_SID:
	case AVTP_IECIIDC_FIELD_CIP_DBS:
	case AVTP_IECIIDC_FIELD_CIP_FN:
	case AVTP_IECIIDC_FIELD_CIP_QPC:
	case AVTP_IECIIDC_FIELD_CIP_SPH:
	case AVTP_IECIIDC_FIELD_CIP_DBC:
	case AVTP_IECIIDC_FIELD_CIP_FMT:
	case AVTP_IECIIDC_FIELD_CIP_TSF:
	case AVTP_IECIIDC_FIELD_CIP_EVT:
	case AVTP_IECIIDC_FIELD_CIP_SFC:
	case AVTP_IECIIDC_FIELD_CIP_N:
	case AVTP_IECIIDC_FIELD_CIP_ND:
	case AVTP_IECIIDC_FIELD_CIP_NO_DATA:
	case AVTP_IECIIDC_FIELD_CIP_SYT:
		res = set_field_value(pdu, field, value);
		break;
	case AVTP_IECIIDC_FIELD_GATEWAY_INFO:
		pdu->format_specific = htonl(value);
		res = 0;
		break;
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

int avtp_ieciidc_pdu_init(struct avtp_stream_pdu *pdu, uint8_t tag)
{
	int res;

	if (!pdu || tag > 0x01)
		return -EINVAL;

	memset(pdu, 0, sizeof(struct avtp_stream_pdu));

	res = avtp_pdu_set((struct avtp_common_pdu *) pdu, AVTP_FIELD_SUBTYPE,
						AVTP_SUBTYPE_61883_IIDC);
	if (res < 0)
		return res;

	res = avtp_stream_pdu_set((struct avtp_stream_pdu *) pdu,
						AVTP_STREAM_FIELD_SV, 1);
	if (res < 0)
		return res;

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_TCODE, 0x0A);
	if (res < 0)
		return res;

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_TAG, tag);
	if (res < 0)
		return res;

	return 0;
}
