/*-
 * Copyright (c) 2003, 2004, 2006 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#include <asn_internal.h>
#include <PrintableString.h>

/*
 * ASN.1:1984 (X.409)
 */
static const int _PrintableString_alphabet[256] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/*                  */
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/*                  */
 1, 0, 0, 0, 0, 0, 0, 2, 3, 4, 0, 5, 6, 7, 8, 9,	/* .      '() +,-./ */
10,11,12,13,14,15,16,17,18,19,20, 0, 0,21, 0,22,	/* 0123456789:  = ? */
 0,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,	/*  ABCDEFGHIJKLMNO */
38,39,40,41,42,43,44,45,46,47,48, 0, 0, 0, 0, 0,	/* PQRSTUVWXYZ      */
 0,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,	/*  abcdefghijklmno */
64,65,66,67,68,69,70,71,72,73,74, 0, 0, 0, 0, 0,	/* pqrstuvwxyz      */
};
static const int _PrintableString_code2value[74] = {
32,39,40,41,43,44,45,46,47,48,49,50,51,52,53,54,
55,56,57,58,61,63,65,66,67,68,69,70,71,72,73,74,
75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,
97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
113,114,115,116,117,118,119,120,121,122};

/*
 * PrintableString basic type description.
 */
static const ber_tlv_tag_t asn_DEF_PrintableString_e2ap_v3_01_tags[] = {
    (ASN_TAG_CLASS_UNIVERSAL | (19 << 2)),  /* [UNIVERSAL 19] IMPLICIT ...*/
    (ASN_TAG_CLASS_UNIVERSAL | (4 << 2))    /* ... OCTET STRING */
};
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static int asn_DEF_PrintableString_e2ap_v3_01_v2c(unsigned int value) {
    return _PrintableString_alphabet[value > 255 ? 0 : value] - 1;
}
static int asn_DEF_PrintableString_e2ap_v3_01_c2v(unsigned int code) {
    if(code < 74)
        return _PrintableString_code2value[code];
    return -1;
}
static asn_per_constraints_t asn_DEF_PrintableString_e2ap_v3_01_per_constraints = {
    { APC_CONSTRAINED, 4, 4, 0x20, 0x39 },   /* Value */
    { APC_SEMI_CONSTRAINED, -1, -1, 0, 0 },  /* Size */
    asn_DEF_PrintableString_e2ap_v3_01_v2c,
    asn_DEF_PrintableString_e2ap_v3_01_c2v
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
asn_TYPE_operation_t asn_OP_PrintableString_e2ap_v3_01 = {
    OCTET_STRING_free_e2ap_v3_01,
#if !defined(ASN_DISABLE_PRINT_SUPPORT)
    OCTET_STRING_print_e2ap_v3_01_utf8,  /* ASCII subset */
#else
    0,
#endif  /* !defined(ASN_DISABLE_PRINT_SUPPORT) */
    OCTET_STRING_compare_e2ap_v3_01,
#if !defined(ASN_DISABLE_BER_SUPPORT)
    OCTET_STRING_decode_ber_e2ap_v3_01,  /* Implemented in terms of OCTET STRING */
    OCTET_STRING_encode_der_e2ap_v3_01,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_BER_SUPPORT) */
#if !defined(ASN_DISABLE_XER_SUPPORT)
    OCTET_STRING_decode_xer_utf8_e2ap_v3_01,
    OCTET_STRING_encode_xer_e2ap_v3_01_utf8,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_XER_SUPPORT) */
#if !defined(ASN_DISABLE_JER_SUPPORT)
    OCTET_STRING_encode_jer_e2ap_v3_01_utf8,
#else
    0,
#endif  /* !defined(ASN_DISABLE_JER_SUPPORT) */
#if !defined(ASN_DISABLE_OER_SUPPORT)
    OCTET_STRING_decode_oer,
    OCTET_STRING_encode_oer,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT)
    OCTET_STRING_decode_uper_e2ap_v3_01,
    OCTET_STRING_encode_uper_e2ap_v3_01,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) */
#if !defined(ASN_DISABLE_APER_SUPPORT)
    OCTET_STRING_decode_aper_e2ap_v3_01,
    OCTET_STRING_encode_aper_e2ap_v3_01,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_RFILL_SUPPORT)
    OCTET_STRING_random_fill_e2ap_v3_01,
#else
    0,
#endif  /* !defined(ASN_DISABLE_RFILL_SUPPORT) */
    0  /* Use generic outmost tag fetcher */
};
asn_TYPE_descriptor_t asn_DEF_PrintableString_e2ap_v3_01 = {
    "PrintableString",
    "PrintableString",
    &asn_OP_PrintableString_e2ap_v3_01,
    asn_DEF_PrintableString_e2ap_v3_01_tags,
    sizeof(asn_DEF_PrintableString_e2ap_v3_01_tags)
      / sizeof(asn_DEF_PrintableString_e2ap_v3_01_tags[0]) - 1,
    asn_DEF_PrintableString_e2ap_v3_01_tags,
    sizeof(asn_DEF_PrintableString_e2ap_v3_01_tags)
      / sizeof(asn_DEF_PrintableString_e2ap_v3_01_tags[0]),
    {
#if !defined(ASN_DISABLE_OER_SUPPORT)
        0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
        &asn_DEF_PrintableString_e2ap_v3_01_per_constraints,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
        PrintableString_constraint_e2ap_v3_01
    },
    0, 0,  /* No members */
    0  /* No specifics */
};

int
PrintableString_constraint_e2ap_v3_01(const asn_TYPE_descriptor_t *td, const void *sptr,
                           asn_app_constraint_failed_f *ctfailcb,
                           void *app_key) {
    const PrintableString_t *st = (const PrintableString_t *)sptr;

	if(st && st->buf) {
		uint8_t *buf = st->buf;
		uint8_t *end = buf + st->size;

		/*
		 * Check the alphabet of the PrintableString.
		 * ASN.1:1984 (X.409)
		 */
		for(; buf < end; buf++) {
			if(!_PrintableString_alphabet[*buf]) {
				ASN__CTFAIL(app_key, td, sptr,
					"%s: value byte %ld (%d) "
					"not in PrintableString alphabet "
					"(%s:%d)",
					td->name,
					(long)((buf - st->buf) + 1),
					*buf,
					__FILE__, __LINE__);
				return -1;
			}
		}
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}

	return 0;
}
