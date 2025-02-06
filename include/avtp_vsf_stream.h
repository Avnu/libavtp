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
enum avtp_vsf_stream_field {
	AVTP_VSF_STREAM_FIELD_SV,
	AVTP_VSF_STREAM_FIELD_MR,
	AVTP_VSF_STREAM_FIELD_TV,
	AVTP_VSF_STREAM_FIELD_SEQ_NUM,
	AVTP_VSF_STREAM_FIELD_TU,
	AVTP_VSF_STREAM_FIELD_STREAM_ID,
	AVTP_VSF_STREAM_FIELD_TIMESTAMP,
	AVTP_VSF_STREAM_FIELD_STREAM_DATA_LEN,
	AVTP_VSF_STREAM_FIELD_VENDOR_ID,
	AVTP_VSF_STREAM_FIELD_MAX,
};

/* Get value of VSF_STREAM AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be retrieved.
 * @val: Pointer to variable which the retrieved value should be saved.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_vsf_stream_pdu_get(const struct avtp_stream_pdu *pdu,
		     enum avtp_vsf_stream_field field, uint64_t *val);

/* Set value of VSF_STREAM AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be set.
 * @val: Value to be set.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_vsf_stream_pdu_set(struct avtp_stream_pdu *pdu, enum avtp_vsf_stream_field field,
		     uint64_t val);

/* Initialize VSF_STREAM AVTPDU. All AVTPDU fields are initialized with zero except
 * 'subtype' (which is set to AVTP_SUBTYPE_VSF_STREAM), 'sv' (which is set to 1),
 * @pdu: Pointer to PDU struct.
 * @subtype: AVTP CVF Format Subtype of this AVTPDU.
 *
 * Return values:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_vsf_stream_pdu_init(struct avtp_stream_pdu *pdu);

#ifdef __cplusplus
}
#endif
