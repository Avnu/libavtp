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

#include <alloca.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>
#include <arpa/inet.h>

#include "avtp.h"
#include "avtp_ieciidc.h"

#define IECIIDC_PDU_HEADER_SIZE (sizeof(struct avtp_stream_pdu) + \
				sizeof(struct avtp_ieciidc_cip_payload))
#define IECIIDC_PDU_HEADER_SPH_SIZE (IECIIDC_PDU_HEADER_SIZE + \
							sizeof(uint32_t))

static void ieciidc_get_field_null_pdu(void **state)
{
	int res;
	uint64_t val = 1;

	res = avtp_ieciidc_pdu_get(NULL, AVTP_IECIIDC_FIELD_GV, &val);

	assert_int_equal(res, -EINVAL);
}

static void ieciidc_get_field_null_val(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_GV, NULL);

	assert_int_equal(res, -EINVAL);
}

static void ieciidc_get_field_invalid_field(void **state)
{
	int res;
	uint64_t val = 1;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_MAX, &val);

	assert_int_equal(res, -EINVAL);
}

static void ieciidc_get_field_sv(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'sv' field to 1. */
	pdu.subtype_data = htonl(0x00800000);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_SV, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void ieciidc_get_field_mr(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'mr' field to 1. */
	pdu.subtype_data = htonl(0x00080000);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_MR, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void ieciidc_get_field_tv(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'tv' field to 1. */
	pdu.subtype_data = htonl(0x00010000);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_TV, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void ieciidc_get_field_seq_num(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'sequence_num' field to 0x55. */
	pdu.subtype_data = htonl(0x00005500);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_SEQ_NUM, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0x55);
}

static void ieciidc_get_field_tu(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'tu' field to 1. */
	pdu.subtype_data = htonl(0x00000001);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_TU, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void ieciidc_get_field_stream_id(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'stream_id' field to 0xAABBCCDDEEFF0001. */
	pdu.stream_id = htobe64(0xAABBCCDDEEFF0001);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_STREAM_ID, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0xAABBCCDDEEFF0001);
}

static void ieciidc_get_field_timestamp(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'avtp_timestamp' field to 0x80C0FFEE. */
	pdu.avtp_time = htonl(0x80C0FFEE);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_TIMESTAMP, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0x80C0FFEE);
}

static void ieciidc_get_field_data_len(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'stream_data_length' field to 0xAAAA. */
	pdu.packet_info = htonl(0xAAAA0000);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_STREAM_DATA_LEN,
									&val);

	assert_int_equal(res, 0);
	assert_true(val == 0xAAAA);
}

static void ieciidc_get_field_gv(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'gv' field to 1. */
	pdu.subtype_data = htonl(0x00020000);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_GV, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0x1);
}

static void ieciidc_get_field_gateway_info(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'gateway info' field to 0x80C0FFEE. */
	pdu.format_specific = htonl(0x80C0FFEE);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_GATEWAY_INFO, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0x80C0FFEE);
}

static void ieciidc_get_field_tag(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'tag' field to 1. */
	pdu.packet_info = htonl(0x00004000);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_TAG, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0x1);
}

static void ieciidc_get_field_channel(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'channel' field to 42. */
	pdu.packet_info = htonl(0x00002A00);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_CHANNEL, &val);

	assert_int_equal(res, 0);
	assert_true(val == 42);
}

static void ieciidc_get_field_tcode(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'tcode' field to 10. */
	pdu.packet_info = htonl(0x000000A0);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_TCODE, &val);

	assert_int_equal(res, 0);
	assert_true(val == 10);
}

static void ieciidc_get_field_sy(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* Set 'sy' field to 10. */
	pdu.packet_info = htonl(0x0000000A);

	res = avtp_ieciidc_pdu_get(&pdu, AVTP_IECIIDC_FIELD_SY, &val);

	assert_int_equal(res, 0);
	assert_true(val == 10);
}

static void ieciidc_get_field_qi_1(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'qi_1' field to 2. */
	pay->cip_1 = htonl(0x80000000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_QI_1, &val);

	assert_int_equal(res, 0);
	assert_true(val == 2);
}

static void ieciidc_get_field_qi_2(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'qi_2' field to 2. */
	pay->cip_2 = htonl(0x80000000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_QI_2, &val);

	assert_int_equal(res, 0);
	assert_true(val == 2);
}

static void ieciidc_get_field_sid(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'sid' field to 42. */
	pay->cip_1 = htonl(0x2A000000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_SID, &val);

	assert_int_equal(res, 0);
	assert_true(val == 42);
}

static void ieciidc_get_field_dbs(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'dbs' field to 0xAA. */
	pay->cip_1 = htonl(0x00AA0000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_DBS, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0xAA);
}

static void ieciidc_get_field_fn(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'fn' field to 2. */
	pay->cip_1 = htonl(0x00008000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_FN, &val);

	assert_int_equal(res, 0);
	assert_true(val == 2);
}

static void ieciidc_get_field_qpc(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'qpc' field to 5. */
	pay->cip_1 = htonl(0x00002800);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_QPC, &val);

	assert_int_equal(res, 0);
	assert_true(val == 5);
}

static void ieciidc_get_field_sph(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'sph' field to 1. */
	pay->cip_1 = htonl(0x0000400);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_SPH, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void ieciidc_get_field_dbc(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'dbc' field to 0xAA. */
	pay->cip_1 = htonl(0x000000AA);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_DBC, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0xAA);
}

static void ieciidc_get_field_fmt(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'fmt' field to 42. */
	pay->cip_2 = htonl(0x2A000000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_FMT, &val);

	assert_int_equal(res, 0);
	assert_true(val == 42);
}

static void ieciidc_get_field_syt(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'syt' field to 0xAAAA. */
	pay->cip_2 = htonl(0x0000AAAA);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_SYT, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0xAAAA);
}

static void ieciidc_get_field_tsf(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'tsf' field to 1. */
	pay->cip_2 = htonl(0x00800000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_TSF, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void ieciidc_get_field_evt(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'evt' field to 2. */
	pay->cip_2 = htonl(0x00200000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_EVT, &val);

	assert_int_equal(res, 0);
	assert_true(val == 2);
}

static void ieciidc_get_field_sfc(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'sfc' field to 5. */
	pay->cip_2 = htonl(0x00050000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_SFC, &val);

	assert_int_equal(res, 0);
	assert_true(val == 5);
}

static void ieciidc_get_field_n(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'n' field to 1. */
	pay->cip_2 = htonl(0x00080000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_N, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void ieciidc_get_field_nd(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'nd' field to 1. */
	pay->cip_2 = htonl(0x00800000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_ND, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void ieciidc_get_field_no_data(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	/* Set 'no_data' field to 0xFF. */
	pay->cip_2 = htonl(0x00FF0000);

	res = avtp_ieciidc_pdu_get(pdu, AVTP_IECIIDC_FIELD_CIP_NO_DATA, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0xFF);
}

static void ieciidc_set_field_null_pdu(void **state)
{
	int res;

	res = avtp_ieciidc_pdu_set(NULL, AVTP_IECIIDC_FIELD_SV, 1);

	assert_int_equal(res, -EINVAL);
}

static void ieciidc_set_field_invalid_field(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_MAX, 1);

	assert_int_equal(res, -EINVAL);
}

static void ieciidc_set_field_sv(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_SV, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00800000);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_mr(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_MR, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00080000);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_tv(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_TV, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00010000);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_seq_num(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_SEQ_NUM, 0x55);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00005500);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_tu(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_TU, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00000001);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_stream_id(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_STREAM_ID,
							0xAABBCCDDEEFF0001);

	assert_int_equal(res, 0);
	assert_true(be64toh(pdu.stream_id) == 0xAABBCCDDEEFF0001);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_timestamp(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_TIMESTAMP,
								0x80C0FFEE);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.avtp_time) == 0x80C0FFEE);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_data_len(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_STREAM_DATA_LEN,
									0xAAAA);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.packet_info) == 0xAAAA0000);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
}

static void ieciidc_set_field_gv(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_GV, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00020000);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_gateway_info(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_GATEWAY_INFO,
								0x80C0FFEE);

	assert_int_equal(res, 0);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(ntohl(pdu.format_specific) == 0x80C0FFEE);
	assert_true(pdu.packet_info == 0);
}

static void ieciidc_set_field_tag(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_TAG, 2);

	assert_int_equal(res, 0);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(ntohl(pdu.packet_info) == 0x00008000);
}

static void ieciidc_set_field_channel(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_CHANNEL, 42);

	assert_int_equal(res, 0);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(ntohl(pdu.packet_info) == 0x00002A00);
}

static void ieciidc_set_field_tcode(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_TCODE, 10);

	assert_int_equal(res, 0);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(ntohl(pdu.packet_info) == 0x000000A0);
}

static void ieciidc_set_field_sy(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_set(&pdu, AVTP_IECIIDC_FIELD_SY, 10);

	assert_int_equal(res, 0);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(ntohl(pdu.packet_info) == 0x0000000A);
}

static void ieciidc_set_field_qi_1(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_QI_1, 2);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(ntohl(pay->cip_1) == 0x80000000);
	assert_true(pay->cip_2 == 0);
}

static void ieciidc_set_field_qi_2(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_QI_2, 2);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x80000000);
}

static void ieciidc_set_field_sid(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_SID, 42);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(ntohl(pay->cip_1) == 0x2A000000);
	assert_true(pay->cip_2 == 0);
}

static void ieciidc_set_field_dbs(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_DBS, 0xAA);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(ntohl(pay->cip_1) == 0x00AA0000);
	assert_true(pay->cip_2 == 0);
}

static void ieciidc_set_field_fn(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_FN, 2);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(ntohl(pay->cip_1) == 0x00008000);
	assert_true(pay->cip_2 == 0);
}

static void ieciidc_set_field_qpc(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_QPC, 5);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(ntohl(pay->cip_1) == 0x00002800);
	assert_true(pay->cip_2 == 0);
}

static void ieciidc_set_field_sph(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_SPH, 1);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(ntohl(pay->cip_1) == 0x00000400);
	assert_true(pay->cip_2 == 0);
}

static void ieciidc_set_field_dbc(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_DBC, 0xAA);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(ntohl(pay->cip_1) == 0x000000AA);
	assert_true(pay->cip_2 == 0);
}

static void ieciidc_set_field_fmt(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_FMT, 42);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x2A000000);
}

static void ieciidc_set_field_syt(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_SYT, 0xAAAA);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x0000AAAA);
}

static void ieciidc_set_field_tsf(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_TSF, 1);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x00800000);
}

static void ieciidc_set_field_evt(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_EVT, 2);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x00200000);
}

static void ieciidc_set_field_sfc(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_SFC, 5);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x00050000);
}

static void ieciidc_set_field_n(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_N, 1);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x00080000);
}

static void ieciidc_set_field_nd(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_ND, 1);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x00800000);
}

static void ieciidc_set_field_no_data(void **state)
{
	int res;
	struct avtp_stream_pdu *pdu = alloca(IECIIDC_PDU_HEADER_SIZE);
	struct avtp_ieciidc_cip_payload *pay =
			(struct avtp_ieciidc_cip_payload *) &pdu->avtp_payload;

	memset(pdu, 0, IECIIDC_PDU_HEADER_SIZE);

	res = avtp_ieciidc_pdu_set(pdu, AVTP_IECIIDC_FIELD_CIP_NO_DATA, 0xFF);

	assert_int_equal(res, 0);
	assert_true(pdu->subtype_data == 0);
	assert_true(pdu->stream_id == 0);
	assert_true(pdu->avtp_time == 0);
	assert_true(pdu->format_specific == 0);
	assert_true(pdu->packet_info == 0);
	assert_true(pay->cip_1 == 0);
	assert_true(ntohl(pay->cip_2) == 0x00FF0000);
}

static void ieciidc_pdu_init_null_pdu(void **state)
{
	int res;

	res = avtp_ieciidc_pdu_init(NULL, 0);

	assert_int_equal(res, -EINVAL);
}

static void ieciidc_pdu_init_invalid_tag(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_ieciidc_pdu_init(&pdu, 0x02);

	assert_int_equal(res, -EINVAL);
}

static void ieciidc_pdu_init(void **state)
{
	int res;
	struct avtp_stream_pdu pdu;

	res = avtp_ieciidc_pdu_init(&pdu, 0x01);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00800000);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(ntohl(pdu.packet_info) == 0x000040A0);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(ieciidc_get_field_null_pdu),
		cmocka_unit_test(ieciidc_get_field_null_val),
		cmocka_unit_test(ieciidc_get_field_invalid_field),
		cmocka_unit_test(ieciidc_get_field_sv),
		cmocka_unit_test(ieciidc_get_field_mr),
		cmocka_unit_test(ieciidc_get_field_tv),
		cmocka_unit_test(ieciidc_get_field_seq_num),
		cmocka_unit_test(ieciidc_get_field_tu),
		cmocka_unit_test(ieciidc_get_field_stream_id),
		cmocka_unit_test(ieciidc_get_field_timestamp),
		cmocka_unit_test(ieciidc_get_field_data_len),
		cmocka_unit_test(ieciidc_get_field_gv),
		cmocka_unit_test(ieciidc_get_field_gateway_info),
		cmocka_unit_test(ieciidc_get_field_tag),
		cmocka_unit_test(ieciidc_get_field_channel),
		cmocka_unit_test(ieciidc_get_field_tcode),
		cmocka_unit_test(ieciidc_get_field_sy),
		cmocka_unit_test(ieciidc_get_field_qi_1),
		cmocka_unit_test(ieciidc_get_field_qi_2),
		cmocka_unit_test(ieciidc_get_field_sid),
		cmocka_unit_test(ieciidc_get_field_dbs),
		cmocka_unit_test(ieciidc_get_field_fn),
		cmocka_unit_test(ieciidc_get_field_qpc),
		cmocka_unit_test(ieciidc_get_field_sph),
		cmocka_unit_test(ieciidc_get_field_dbc),
		cmocka_unit_test(ieciidc_get_field_fmt),
		cmocka_unit_test(ieciidc_get_field_syt),
		cmocka_unit_test(ieciidc_get_field_tsf),
		cmocka_unit_test(ieciidc_get_field_evt),
		cmocka_unit_test(ieciidc_get_field_sfc),
		cmocka_unit_test(ieciidc_get_field_n),
		cmocka_unit_test(ieciidc_get_field_nd),
		cmocka_unit_test(ieciidc_get_field_no_data),
		cmocka_unit_test(ieciidc_set_field_null_pdu),
		cmocka_unit_test(ieciidc_set_field_invalid_field),
		cmocka_unit_test(ieciidc_set_field_sv),
		cmocka_unit_test(ieciidc_set_field_mr),
		cmocka_unit_test(ieciidc_set_field_tv),
		cmocka_unit_test(ieciidc_set_field_seq_num),
		cmocka_unit_test(ieciidc_set_field_tu),
		cmocka_unit_test(ieciidc_set_field_stream_id),
		cmocka_unit_test(ieciidc_set_field_timestamp),
		cmocka_unit_test(ieciidc_set_field_data_len),
		cmocka_unit_test(ieciidc_set_field_gv),
		cmocka_unit_test(ieciidc_set_field_gateway_info),
		cmocka_unit_test(ieciidc_set_field_tag),
		cmocka_unit_test(ieciidc_set_field_channel),
		cmocka_unit_test(ieciidc_set_field_tcode),
		cmocka_unit_test(ieciidc_set_field_sy),
		cmocka_unit_test(ieciidc_set_field_qi_1),
		cmocka_unit_test(ieciidc_set_field_qi_2),
		cmocka_unit_test(ieciidc_set_field_sid),
		cmocka_unit_test(ieciidc_set_field_dbs),
		cmocka_unit_test(ieciidc_set_field_fn),
		cmocka_unit_test(ieciidc_set_field_qpc),
		cmocka_unit_test(ieciidc_set_field_sph),
		cmocka_unit_test(ieciidc_set_field_dbc),
		cmocka_unit_test(ieciidc_set_field_fmt),
		cmocka_unit_test(ieciidc_set_field_syt),
		cmocka_unit_test(ieciidc_set_field_tsf),
		cmocka_unit_test(ieciidc_set_field_evt),
		cmocka_unit_test(ieciidc_set_field_sfc),
		cmocka_unit_test(ieciidc_set_field_n),
		cmocka_unit_test(ieciidc_set_field_nd),
		cmocka_unit_test(ieciidc_set_field_no_data),
		cmocka_unit_test(ieciidc_pdu_init_null_pdu),
		cmocka_unit_test(ieciidc_pdu_init_invalid_tag),
		cmocka_unit_test(ieciidc_pdu_init),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
