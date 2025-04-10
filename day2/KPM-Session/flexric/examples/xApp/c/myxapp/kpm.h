 
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


bool eq_sm(sm_ran_function_t const* elem, int const id);
size_t find_sm_idx(sm_ran_function_t* rf, size_t sz, bool (*f)(sm_ran_function_t const*, int const), int const id);
kpm_sub_data_t gen_kpm_subs(kpm_ran_function_def_t const* ran_func,int value);
kpm_act_def_t fill_report_style_4(ric_report_style_item_t const* report_item);
test_info_lst_t filter_predicate(test_cond_type_e type, test_cond_e cond, int value);
label_info_lst_t fill_kpm_label(void);
kpm_act_def_format_1_t fill_act_def_frm_1(ric_report_style_item_t const* report_item);
void sm_cb_kpm(sm_ag_if_rd_t const* rd);
void log_kpm_measurements(kpm_ind_msg_format_1_t const* msg_frm_1);
void match_meas_name_type(meas_type_t meas_type, meas_record_lst_t meas_record);
void match_id_meas_type(meas_type_t meas_type, meas_record_lst_t meas_record);
void log_real_value(byte_array_t name, meas_record_lst_t meas_record);
void log_int_value(byte_array_t name, meas_record_lst_t meas_record);
void log_cuup_ue_id(ue_id_e2sm_t ue_id);
void log_du_ue_id(ue_id_e2sm_t ue_id);
void log_gnb_ue_id(ue_id_e2sm_t ue_id);

typedef kpm_act_def_t (*fill_kpm_act_def)(ric_report_style_item_t const* report_item);



typedef void (*check_meas_type)(meas_type_t meas_type, meas_record_lst_t meas_record);



typedef void (*log_meas_value)(byte_array_t name, meas_record_lst_t meas_record);


typedef void (*log_ue_id)(ue_id_e2sm_t ue_id);

// Declare the arrays
extern log_ue_id log_ue_id_e2sm_S[END_UE_ID_E2SM];
extern log_meas_value get_meas_value_S[END_MEAS_VALUE];
extern check_meas_type match_meas_type_S[END_MEAS_TYPE];
extern fill_kpm_act_def get_kpm_act_def_S[END_RIC_SERVICE_REPORT];


#endif 