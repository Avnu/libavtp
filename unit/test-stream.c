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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <arpa/inet.h>

#include "avtp.h"
#include "avtp_stream.h"

static void stream_get_field_null_pdu(void **state)
{
	int res;
	uint64_t val;

	res = avtp_stream_pdu_get(NULL, AVTP_STREAM_FIELD_SV, &val);

	assert_int_equal(res, -EINVAL);
}

static void stream_get_field_null_val(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_SV, NULL);

	assert_int_equal(res, -EINVAL);
}

static void stream_get_field_invalid_field(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_MAX, &val);

	assert_int_equal(res, -EINVAL);
}

static void stream_get_field_sv(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* stream_set 'sv' field to 1. */
	pdu.subtype_data = htonl(0x00800000);

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_SV, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void stream_get_field_mr(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* stream_set 'mr' field to 1. */
	pdu.subtype_data = htonl(0x00080000);

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_MR, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void stream_get_field_tv(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* stream_set 'tv' field to 1. */
	pdu.subtype_data = htonl(0x00010000);

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_TV, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void stream_get_field_seq_num(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* stream_set 'sequence_num' field to 0x55. */
	pdu.subtype_data = htonl(0x00005500);

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_SEQ_NUM, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0x55);
}

static void stream_get_field_tu(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* stream_set 'tu' field to 1. */
	pdu.subtype_data = htonl(0x00000001);

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_TU, &val);

	assert_int_equal(res, 0);
	assert_true(val == 1);
}

static void stream_get_field_stream_id(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* stream_set 'stream_id' field to 0xAABBCCDDEEFF0001. */
	pdu.stream_id = htobe64(0xAABBCCDDEEFF0001);

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_STREAM_ID, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0xAABBCCDDEEFF0001);
}

static void stream_get_field_timestamp(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* stream_set 'avtp_timestamp' field to 0x80C0FFEE. */
	pdu.avtp_time = htonl(0x80C0FFEE);

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_TIMESTAMP, &val);

	assert_int_equal(res, 0);
	assert_true(val == 0x80C0FFEE);
}

static void stream_get_field_data_len(void **state)
{
	int res;
	uint64_t val;
	struct avtp_stream_pdu pdu = { 0 };

	/* stream_set 'stream_data_length' field to 0xAAAA. */
	pdu.packet_info = htonl(0xAAAA0000);

	res = avtp_stream_pdu_get(&pdu, AVTP_STREAM_FIELD_STREAM_DATA_LEN,
									&val);

	assert_int_equal(res, 0);
	assert_true(val == 0xAAAA);
}

static void stream_set_field_null_pdu(void **state)
{
	int res;

	res = avtp_stream_pdu_set(NULL, AVTP_STREAM_FIELD_SV, 0);

	assert_int_equal(res, -EINVAL);
}

static void stream_set_field_invalid_field(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_MAX, 1);

	assert_int_equal(res, -EINVAL);
}

static void stream_set_field_sv(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_SV, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00800000);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void stream_set_field_mr(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_MR, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00080000);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void stream_set_field_tv(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_TV, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00010000);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void stream_set_field_seq_num(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_SEQ_NUM, 0x55);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00005500);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void stream_set_field_tu(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_TU, 1);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.subtype_data) == 0x00000001);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void stream_set_field_stream_id(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_STREAM_ID,
							0xAABBCCDDEEFF0001);

	assert_int_equal(res, 0);
	assert_true(be64toh(pdu.stream_id) == 0xAABBCCDDEEFF0001);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void stream_set_field_timestamp(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_TIMESTAMP,
								0x80C0FFEE);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.avtp_time) == 0x80C0FFEE);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.format_specific == 0);
	assert_true(pdu.packet_info == 0);
}

static void stream_set_field_data_len(void **state)
{
	int res;
	struct avtp_stream_pdu pdu = { 0 };

	res = avtp_stream_pdu_set(&pdu, AVTP_STREAM_FIELD_STREAM_DATA_LEN,
									0xAAAA);

	assert_int_equal(res, 0);
	assert_true(ntohl(pdu.packet_info) == 0xAAAA0000);
	assert_true(pdu.subtype_data == 0);
	assert_true(pdu.stream_id == 0);
	assert_true(pdu.avtp_time == 0);
	assert_true(pdu.format_specific == 0);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(stream_get_field_null_pdu),
		cmocka_unit_test(stream_get_field_null_val),
		cmocka_unit_test(stream_get_field_invalid_field),
		cmocka_unit_test(stream_get_field_sv),
		cmocka_unit_test(stream_get_field_mr),
		cmocka_unit_test(stream_get_field_tv),
		cmocka_unit_test(stream_get_field_seq_num),
		cmocka_unit_test(stream_get_field_tu),
		cmocka_unit_test(stream_get_field_stream_id),
		cmocka_unit_test(stream_get_field_timestamp),
		cmocka_unit_test(stream_get_field_data_len),
		cmocka_unit_test(stream_set_field_null_pdu),
		cmocka_unit_test(stream_set_field_invalid_field),
		cmocka_unit_test(stream_set_field_sv),
		cmocka_unit_test(stream_set_field_mr),
		cmocka_unit_test(stream_set_field_tv),
		cmocka_unit_test(stream_set_field_seq_num),
		cmocka_unit_test(stream_set_field_tu),
		cmocka_unit_test(stream_set_field_stream_id),
		cmocka_unit_test(stream_set_field_timestamp),
		cmocka_unit_test(stream_set_field_data_len),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
