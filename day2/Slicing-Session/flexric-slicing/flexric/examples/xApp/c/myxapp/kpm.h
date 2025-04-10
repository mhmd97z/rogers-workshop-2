
#ifndef KPM_H
#define KPM_H

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../../../../src/util/e.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>



extern pthread_mutex_t mtx;
extern FILE *file;
extern FILE *csv_file;

// Define a struct for existing entries (for comparison)
typedef struct {
    uint64_t amf_ue_ngap_id;
    uint32_t mcc;
    uint32_t mnc;
    uint32_t mnc_digit_len;
    uint32_t amf_region_id;
    uint32_t amf_set_id;
    uint32_t amf_ptr;
} ue_entry_t;

// Define a struct for KPM subscription data
// Featurses: "KPM ind_msg latency,UE ID type,gNB,amf_ue_ngap_id,ran_ue_id,DRB.PdcpSduVolumeDL,DRB.PdcpSduVolumeUL,DRB.RlcSduDelayDl,DRB.UEThpDl,DRB.UEThpUl,RRU.PrbTotDl,RRU.PrbTotUl
typedef struct {
    uint64_t latency;
    uint32_t ue_id_type;
    uint64_t amf_ue_ngap_id;
    uint64_t ran_ue_id;
    uint32_t pdcp_sdu_volume_dl;
    uint32_t pdcp_sdu_volume_ul;
    double rlc_sdu_delay_dl;
    double ue_thp_dl;
    double ue_thp_ul;
    uint32_t prb_tot_dl;
    uint32_t prb_tot_ul;
    uint32_t slice_id;
} kpm_entry_t;


bool eq_sm(sm_ran_function_t const* elem, int const id);
size_t find_sm_idx(sm_ran_function_t* rf, size_t sz, bool (*f)(sm_ran_function_t const*, int const), int const id);
kpm_sub_data_t gen_kpm_subs(kpm_ran_function_def_t const* ran_func,int value);
kpm_act_def_t fill_report_style_4(ric_report_style_item_t const* report_item);
test_info_lst_t filter_predicate(test_cond_type_e type, test_cond_e cond, int value);
label_info_lst_t fill_kpm_label(void);
kpm_act_def_format_1_t fill_act_def_frm_1(ric_report_style_item_t const* report_item);
void sm_cb_kpm(sm_ag_if_rd_t const* rd);
// void log_kpm_measurements(kpm_ind_msg_format_1_t const* msg_frm_1);
void log_kpm_measurements(kpm_ind_msg_format_1_t const* msg_frm_1, kpm_entry_t* kpm_entry);
// void match_meas_name_type(meas_type_t meas_type, meas_record_lst_t meas_record);
void match_meas_name_type(meas_type_t meas_type, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry);
// void match_id_meas_type(meas_type_t meas_type, meas_record_lst_t meas_record);
void match_id_meas_type(meas_type_t meas_type, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry);
// void log_real_value(byte_array_t name, meas_record_lst_t meas_record);
void log_real_value(byte_array_t name, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry);
// void log_int_value(byte_array_t name, meas_record_lst_t meas_record);
void log_int_value(byte_array_t name, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry);
// void log_cuup_ue_id(ue_id_e2sm_t ue_id);
void log_cuup_ue_id(ue_id_e2sm_t ue_id, kpm_entry_t* kpm_entry);
// void log_du_ue_id(ue_id_e2sm_t ue_id);
void log_du_ue_id(ue_id_e2sm_t ue_id, kpm_entry_t* kpm_entry);
// void log_gnb_ue_id(ue_id_e2sm_t ue_id);
void log_gnb_ue_id(ue_id_e2sm_t ue_id, kpm_entry_t* kpm_entry);

typedef kpm_act_def_t (*fill_kpm_act_def)(ric_report_style_item_t const* report_item);



// typedef void (*check_meas_type)(meas_type_t meas_type, meas_record_lst_t meas_record);
typedef void (*check_meas_type)(meas_type_t meas_type, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry);




// typedef void (*log_meas_value)(byte_array_t name, meas_record_lst_t meas_record);
typedef void (*log_meas_value)(byte_array_t name, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry);



// typedef void (*log_ue_id)(ue_id_e2sm_t ue_id);
typedef void (*log_ue_id)(ue_id_e2sm_t ue_id, kpm_entry_t* kpm_entry);

// Declare the arrays
extern log_ue_id log_ue_id_e2sm_S[END_UE_ID_E2SM];
extern log_meas_value get_meas_value_S[END_MEAS_VALUE];
extern check_meas_type match_meas_type_S[END_MEAS_TYPE];
extern fill_kpm_act_def get_kpm_act_def_S[END_RIC_SERVICE_REPORT];


#endif 