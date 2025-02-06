/*
 * Copyright (c) 2025, Collabora Ltd
 * Olivier Crete <olivier.crete@collabora.com>
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
#include "avtp_vsf_stream.h"
#include "avtp_stream.h"
#include "util.h"

#define SHIFT_VENDOR_ID_1      (31 - 31)
#define SHIFT_VENDOR_ID_2      (31 - 31)

#define MASK_VENDOR_ID_1      (BITMASK(32) << SHIFT_VENDOR_ID_1)
#define MASK_VENDOR_ID_2      (BITMASK(16) << SHIFT_VENDOR_ID_2)

static int get_vendor_id(const struct avtp_stream_pdu *pdu, uint64_t *val)
{
        uint32_t bitmap1, bitmap2;
        uint64_t val1, val2;

        bitmap1 = ntohl(pdu->format_specific);
        bitmap2 = ntohl(pdu->packet_info);

        val1 = BITMAP_GET_VALUE(bitmap1, MASK_VENDOR_ID_1, SHIFT_VENDOR_ID_1);
        val2 = BITMAP_GET_VALUE(bitmap2, MASK_VENDOR_ID_2, SHIFT_VENDOR_ID_2);

        *val = val1 << 16 | val2;

        return 0;
}

int avtp_vsf_stream_pdu_get(const struct avtp_stream_pdu *pdu,
		     enum avtp_vsf_stream_field field, uint64_t *val)
{
	int res;

	if (!pdu || !val)
		return -EINVAL;

	switch (field) {
	case AVTP_VSF_STREAM_FIELD_SV:
	case AVTP_VSF_STREAM_FIELD_MR:
	case AVTP_VSF_STREAM_FIELD_TV:
	case AVTP_VSF_STREAM_FIELD_SEQ_NUM:
	case AVTP_VSF_STREAM_FIELD_TU:
	case AVTP_VSF_STREAM_FIELD_STREAM_DATA_LEN:
	case AVTP_VSF_STREAM_FIELD_TIMESTAMP:
	case AVTP_VSF_STREAM_FIELD_STREAM_ID:
		res = avtp_stream_pdu_get(pdu, (enum avtp_stream_field)field,
					  val);
		break;
	case AVTP_VSF_STREAM_FIELD_VENDOR_ID:
		res = get_vendor_id(pdu, val);
		break;
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

static int set_vendor_id(struct avtp_stream_pdu *pdu, uint64_t val)
{
	uint32_t bitmap;
	void *ptr;

        ptr = &pdu->format_specific;
	bitmap = get_unaligned_be32(ptr);
	BITMAP_SET_VALUE(bitmap, val >> 16, MASK_VENDOR_ID_1, SHIFT_VENDOR_ID_1);
	put_unaligned_be32(bitmap, ptr);

        ptr = &pdu->packet_info;
        bitmap = get_unaligned_be32(ptr);
	BITMAP_SET_VALUE(bitmap, val & 0xFFFF, MASK_VENDOR_ID_2, SHIFT_VENDOR_ID_2);
	put_unaligned_be32(bitmap, ptr);

	return 0;
}

int avtp_vsf_stream_pdu_set(struct avtp_stream_pdu *pdu, enum avtp_vsf_stream_field field,
		     uint64_t val)
{
	int res;

	if (!pdu)
		return -EINVAL;

	switch (field) {
	case AVTP_VSF_STREAM_FIELD_SV:
	case AVTP_VSF_STREAM_FIELD_MR:
	case AVTP_VSF_STREAM_FIELD_TV:
	case AVTP_VSF_STREAM_FIELD_SEQ_NUM:
	case AVTP_VSF_STREAM_FIELD_TU:
	case AVTP_VSF_STREAM_FIELD_STREAM_DATA_LEN:
	case AVTP_VSF_STREAM_FIELD_TIMESTAMP:
	case AVTP_VSF_STREAM_FIELD_STREAM_ID:
		res = avtp_stream_pdu_set(pdu, (enum avtp_stream_field)field,
					  val);
		break;
	case AVTP_VSF_STREAM_FIELD_VENDOR_ID:
		res = set_vendor_id(pdu, val);
		break;
	default:
		res = -EINVAL;
		break;
	}

	return res;
}

int avtp_vsf_stream_pdu_init(struct avtp_stream_pdu *pdu)
{
	int res;

	if (!pdu)
		return -EINVAL;

	memset(pdu, 0, sizeof(struct avtp_stream_pdu));

	res = avtp_pdu_set((struct avtp_common_pdu *)pdu, AVTP_FIELD_SUBTYPE,
			   AVTP_SUBTYPE_VSF_STREAM);
	if (res < 0)
		return res;

	res = avtp_vsf_stream_pdu_set(pdu, AVTP_VSF_STREAM_FIELD_SV, 1);
	if (res < 0)
		return res;

	return 0;
}
