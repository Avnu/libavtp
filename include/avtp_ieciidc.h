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

#pragma once

#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AVTP_IECIIDC_TAG_NO_CIP		0x00
#define AVTP_IECIIDC_TAG_CIP		0x01

enum avtp_ieciidc_field {
	AVTP_IECIIDC_FIELD_SV,
	AVTP_IECIIDC_FIELD_MR,
	AVTP_IECIIDC_FIELD_TV,
	AVTP_IECIIDC_FIELD_SEQ_NUM,
	AVTP_IECIIDC_FIELD_TU,
	AVTP_IECIIDC_FIELD_STREAM_ID,
	AVTP_IECIIDC_FIELD_TIMESTAMP,
	AVTP_IECIIDC_FIELD_STREAM_DATA_LEN,
	AVTP_IECIIDC_FIELD_GV,
	AVTP_IECIIDC_FIELD_GATEWAY_INFO,
	AVTP_IECIIDC_FIELD_TAG,
	AVTP_IECIIDC_FIELD_CHANNEL,
	AVTP_IECIIDC_FIELD_TCODE,
	AVTP_IECIIDC_FIELD_SY,
	AVTP_IECIIDC_FIELD_CIP_QI_1,
	AVTP_IECIIDC_FIELD_CIP_QI_2,
	AVTP_IECIIDC_FIELD_CIP_SID,
	AVTP_IECIIDC_FIELD_CIP_DBS,
	AVTP_IECIIDC_FIELD_CIP_FN,
	AVTP_IECIIDC_FIELD_CIP_QPC,
	AVTP_IECIIDC_FIELD_CIP_SPH,
	AVTP_IECIIDC_FIELD_CIP_DBC,
	AVTP_IECIIDC_FIELD_CIP_FMT,
	AVTP_IECIIDC_FIELD_CIP_SYT,

	/* Fields defined below are the FDF - Format Dependent Field - fields.
	 * As they are poorly described in the IEEE 1722 - it usually refers to
	 * the IEC 61883-1, which in turn refers to each relevant IEC 61883
	 * part, this comment is an attempt to summarize the description of
	 * those fields, as described in IEC 61883.
	 *
	 * Note that FDF is the one defined on Figure 23 of IEEE 1722-2016, and
	 * FDF_3 is the one with 3 octets, seen on Figure 24 of the same
	 * standard.
	 *
	 *  - IEC 61883-4 uses FDF_3, with fields:
	 *    - TSF (Bit 0)
	 *
	 *  - IEC 61883-6 uses FDF, with fields:
	 *    - EVT (Bits 2-3)
	 *    - SFC (Bits 5-7)
	 *    - N   (Bit 4)
	 *    - NO-DATA (All 8 bits set to 1)
	 *
	 *  - IEC 61883-7 uses FDF_3, with fields:
	 *    - TSF (Bit 0)
	 *
	 *  - IEC 61883-8 uses FDF, with fields:
	 *    - ND (Bit 0)
	 */
	AVTP_IECIIDC_FIELD_CIP_TSF,
	AVTP_IECIIDC_FIELD_CIP_EVT,
	AVTP_IECIIDC_FIELD_CIP_SFC,
	AVTP_IECIIDC_FIELD_CIP_N,
	AVTP_IECIIDC_FIELD_CIP_ND,
	AVTP_IECIIDC_FIELD_CIP_NO_DATA,
	AVTP_IECIIDC_FIELD_MAX,
};

struct avtp_ieciidc_cip_payload {
	uint32_t cip_1;
	uint32_t cip_2;
	uint8_t cip_data_payload[0];
};

struct avtp_ieciidc_cip_source_packet {
	uint32_t avtp_source_packet_header_timestamp;
	uint8_t cip_with_sph_payload[0];
};

/* Get value from IEC 61883/IIDC AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be retrieved.
 * @val: Pointer to variable which the retrieved value should be saved.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_ieciidc_pdu_get(const struct avtp_stream_pdu *pdu,
			enum avtp_ieciidc_field field, uint64_t *val);

/* Set value from IEC 61883/IIDC AVTPDU field.
 * @pdu: Pointer to PDU struct.
 * @field: PDU field to be set.
 * @val: Value to be set.
 *
 * Returns:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_ieciidc_pdu_set(struct avtp_stream_pdu *pdu,
				enum avtp_ieciidc_field field, uint64_t val);

/* Initialize IEC 61883/IIDC AVTPDU. The following fields are pre-initialised:
 * 'subtype' -> AVTP_SUBTYPE_61883_IIDC
 * 'sv' -> 0x01
 * 'tcode' -> 0x0A
 * 'tag' -> tag informed on parameter
 * All other fields are set to zero.
 * @pdu: Pointer to PDU struct.
 * @tag: Value of AVTP_IECIIDC_FIELD_TAG.
 *
 * Return values:
 *    0: Success.
 *    -EINVAL: If any argument is invalid.
 */
int avtp_ieciidc_pdu_init(struct avtp_stream_pdu *pdu, uint8_t tag);

#ifdef __cplusplus
}
#endif
