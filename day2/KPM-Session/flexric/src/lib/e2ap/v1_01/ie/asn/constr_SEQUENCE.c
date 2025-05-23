/*
 * Copyright (c) 2003-2017 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#include <asn_internal.h>
#include <constr_SEQUENCE.h>

asn_TYPE_operation_t asn_OP_SEQUENCE_e2ap_v1_01 = {
    SEQUENCE_free_e2ap_v1_01,
#if !defined(ASN_DISABLE_PRINT_SUPPORT)
    SEQUENCE_print_e2ap_v1_01,
#else
    0,
#endif  /* !defined(ASN_DISABLE_PRINT_SUPPORT) */
    SEQUENCE_compare_e2ap_v1_01,
#if !defined(ASN_DISABLE_BER_SUPPORT)
    SEQUENCE_decode_ber_e2ap_v1_01,
    SEQUENCE_encode_der_e2ap_v1_01,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_BER_SUPPORT) */
#if !defined(ASN_DISABLE_XER_SUPPORT)
    SEQUENCE_decode_xer_e2ap_v1_01,
    SEQUENCE_encode_xer_e2ap_v1_01,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_XER_SUPPORT) */
#if !defined(ASN_DISABLE_JER_SUPPORT)
    SEQUENCE_encode_jer,
#else
    0,
#endif  /* !defined(ASN_DISABLE_JER_SUPPORT) */
#if !defined(ASN_DISABLE_OER_SUPPORT)
    SEQUENCE_decode_oer,
    SEQUENCE_encode_oer,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT)
    SEQUENCE_decode_uper_e2ap_v1_01,
    SEQUENCE_encode_uper_e2ap_v1_01,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) */
#if !defined(ASN_DISABLE_APER_SUPPORT)
    SEQUENCE_decode_aper_e2ap_v1_01,
    SEQUENCE_encode_aper_e2ap_v1_01,
#else
    0,
    0,
#endif  /* !defined(ASN_DISABLE_APER_SUPPORT) */
#if !defined(ASN_DISABLE_RFILL_SUPPORT)
    SEQUENCE_random_fill_e2ap_v1_01,
#else
    0,
#endif  /* !defined(ASN_DISABLE_RFILL_SUPPORT) */
    0  /* Use generic outmost tag fetcher */
};

void
SEQUENCE_free_e2ap_v1_01(const asn_TYPE_descriptor_t *td, void *sptr,
              enum asn_struct_free_method method) {
    size_t edx;
    const asn_SEQUENCE_specifics_t *specs =
        (const asn_SEQUENCE_specifics_t *)td->specifics;
    asn_struct_ctx_t *ctx; /* Decoder context */

	if(!td || !sptr)
		return;

	ASN_DEBUG("Freeing %s as SEQUENCE", td->name);

	for(edx = 0; edx < td->elements_count; edx++) {
		asn_TYPE_member_t *elm = &td->elements[edx];
		void *memb_ptr;
		if(elm->flags & ATF_POINTER) {
			memb_ptr = *(void **)((char *)sptr + elm->memb_offset);
			if(memb_ptr)
				ASN_STRUCT_FREE(*elm->type, memb_ptr);
		} else {
			memb_ptr = (void *)((char *)sptr + elm->memb_offset);
			ASN_STRUCT_FREE_CONTENTS_ONLY(*elm->type, memb_ptr);
		}
	}

	/* Clean parsing context */
	ctx = (asn_struct_ctx_t *)((char *)sptr + specs->ctx_offset);
	FREEMEM(ctx->ptr);

    switch(method) {
    case ASFM_FREE_EVERYTHING:
        FREEMEM(sptr);
        break;
    case ASFM_FREE_UNDERLYING:
        break;
    case ASFM_FREE_UNDERLYING_AND_RESET:
        memset(
            sptr, 0,
            ((const asn_SEQUENCE_specifics_t *)(td->specifics))->struct_size);
        break;
    }
}

int
SEQUENCE_constraint_e2ap_v1_01(const asn_TYPE_descriptor_t *td, const void *sptr,
                    asn_app_constraint_failed_f *ctfailcb, void *app_key) {
    size_t edx;

	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Iterate over structure members and check their validity.
	 */
	for(edx = 0; edx < td->elements_count; edx++) {
		asn_TYPE_member_t *elm = &td->elements[edx];
		const void *memb_ptr;
		asn_constr_check_f *constr;
		int ret;

		if(elm->flags & ATF_POINTER) {
			memb_ptr = *(const void * const *)((const char *)sptr + elm->memb_offset);
			if(!memb_ptr) {
				if(elm->optional)
					continue;
				ASN__CTFAIL(app_key, td, sptr,
				"%s: mandatory element %s absent (%s:%d)",
				td->name, elm->name, __FILE__, __LINE__);
				return -1;
			}
		} else {
			memb_ptr = (const void *)((const char *)sptr + elm->memb_offset);
		}

		constr = elm->encoding_constraints.general_constraints;
		if(!constr)
			constr = elm->type->encoding_constraints.general_constraints;

		ret = constr(elm->type, memb_ptr, ctfailcb, app_key);
		if(ret) return ret;
	}

	return 0;
}

int
SEQUENCE_compare_e2ap_v1_01(const asn_TYPE_descriptor_t *td, const void *aptr,
                 const void *bptr) {
    size_t edx;

	for(edx = 0; edx < td->elements_count; edx++) {
		asn_TYPE_member_t *elm = &td->elements[edx];
		const void *amemb;
		const void *bmemb;
        int ret;

		if(elm->flags & ATF_POINTER) {
            amemb =
                *(const void *const *)((const char *)aptr + elm->memb_offset);
            bmemb =
                *(const void *const *)((const char *)bptr + elm->memb_offset);
            if(!amemb) {
                if(!bmemb) continue;
                if(elm->default_value_cmp
                   && elm->default_value_cmp(bmemb) == 0) {
                    /* A is absent, but B is present and equal to DEFAULT */
                    continue;
                }
                return -1;
            } else if(!bmemb) {
                if(elm->default_value_cmp
                   && elm->default_value_cmp(amemb) == 0) {
                    /* B is absent, but A is present and equal to DEFAULT */
                    continue;
                }
                return 1;
            }
		} else {
            amemb = (const void *)((const char *)aptr + elm->memb_offset);
            bmemb = (const void *)((const char *)bptr + elm->memb_offset);
		}

        ret = elm->type->op->compare_struct(elm->type, amemb, bmemb);
        if(ret != 0) return ret;
    }

    return 0;
}
