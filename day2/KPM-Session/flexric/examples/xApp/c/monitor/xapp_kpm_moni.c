/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BAS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

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
#include <yaml.h>
#include <string.h>

// static
// uint64_t const period_ms = 1000;

static
pthread_mutex_t mtx;

FILE *file = NULL;  // Global variable

static void parse_yaml_file(uint64_t *granularity, char ***kpi_list, int *kpi_count) {
  FILE *fh = fopen("/home/user/workshop/scripts/kpm.config.yaml", "r");
  if (!fh) {
    perror("Failed to open YAML file");
    return;
  }

  yaml_parser_t parser;
  yaml_event_t event;

  *granularity = 0;
  int in_kpi_list = 0;
  *kpi_list = malloc(10 * sizeof(char *));

  if (!yaml_parser_initialize(&parser)) {
    fputs("Failed to initialize parser!\n", stderr);
    fclose(fh);
    return;
  }

  yaml_parser_set_input_file(&parser, fh);

  while (1) {
    if (!yaml_parser_parse(&parser, &event)) {
      fprintf(stderr, "Parser error %d\n", parser.error);
      break;
    }

    if (event.type == YAML_STREAM_END_EVENT) {
      yaml_event_delete(&event);
      break;
    }

    if (event.type == YAML_SCALAR_EVENT) {
      if (strcmp((char *)event.data.scalar.value, "period") == 0) {
        yaml_event_delete(&event);
        if (!yaml_parser_parse(&parser, &event)) {
          fprintf(stderr, "Parser error %d\n", parser.error);
          break;
        }
        *granularity = atoi((char *)event.data.scalar.value);
      } else if (strcmp((char *)event.data.scalar.value, "kpi_list") == 0) {
        in_kpi_list = 1;
      } else if (in_kpi_list) {
        (*kpi_list)[*kpi_count] = malloc(100 * sizeof(char *));
        strcpy((*kpi_list)[*kpi_count], (char *)event.data.scalar.value);
        (*kpi_count)++;
      }
    } else if (event.type == YAML_SEQUENCE_END_EVENT && in_kpi_list) {
      in_kpi_list = 0;
    }

    yaml_event_delete(&event);
  }

  yaml_parser_delete(&parser);
  fclose(fh);
}

static
void log_gnb_ue_id(ue_id_e2sm_t ue_id, FILE *file)
{
  if (ue_id.gnb.gnb_cu_ue_f1ap_lst != NULL) {
    for (size_t i = 0; i < ue_id.gnb.gnb_cu_ue_f1ap_lst_len; i++) {
      fprintf(file, "UE ID type,gNB-CU,gnb_cu_ue_f1ap,%u\n", ue_id.gnb.gnb_cu_ue_f1ap_lst[i]);
    }
  } else {
    fprintf(file, "UE ID type,gNB,amf_ue_ngap_id,%lu\n", ue_id.gnb.amf_ue_ngap_id);
  }
  if (ue_id.gnb.ran_ue_id != NULL) {
    fprintf(file, "ran_ue_id,%lx\n", *ue_id.gnb.ran_ue_id);
  }
}

static
void log_du_ue_id(ue_id_e2sm_t ue_id, FILE *file)
{
  (void)file;  // Suppress unused parameter warning
  printf("UE ID type = gNB-DU, gnb_cu_ue_f1ap = %u\n", ue_id.gnb_du.gnb_cu_ue_f1ap);
  if (ue_id.gnb_du.ran_ue_id != NULL) {
    printf("ran_ue_id = %lx\n", *ue_id.gnb_du.ran_ue_id); // RAN UE NGAP ID 
  }
}

static
void log_cuup_ue_id(ue_id_e2sm_t ue_id, FILE *file)
{
  (void)file;  // Suppress unused parameter warning
  printf("UE ID type = gNB-CU-UP, gnb_cu_cp_ue_e1ap = %u\n", ue_id.gnb_cu_up.gnb_cu_cp_ue_e1ap);
  if (ue_id.gnb_cu_up.ran_ue_id != NULL) {
    printf("ran_ue_id = %lx\n", *ue_id.gnb_cu_up.ran_ue_id); // RAN UE NGAP ID
  }
}

typedef void (*log_ue_id)(ue_id_e2sm_t ue_id, FILE *file);

static
log_ue_id log_ue_id_e2sm[END_UE_ID_E2SM] = {
    log_gnb_ue_id, // common for gNB-mono, CU and CU-CP
    log_du_ue_id,
    log_cuup_ue_id,
    NULL,
    NULL,
    NULL,
    NULL,
};


static
void log_int_value(byte_array_t name, meas_record_lst_t meas_record, FILE *file)
{
  if (cmp_str_ba("RRU.PrbTotDl", name) == 0) {
    fprintf(file, "RRU.PrbTotDl,%d,PRBs\n", meas_record.int_val);
  } else if (cmp_str_ba("RRU.PrbTotUl", name) == 0) {
    fprintf(file, "RRU.PrbTotUl,%d,PRBs\n", meas_record.int_val);
  } else if (cmp_str_ba("DRB.PdcpSduVolumeDL", name) == 0) {
    fprintf(file, "DRB.PdcpSduVolumeDL,%d,kb\n", meas_record.int_val);
  } else if (cmp_str_ba("DRB.PdcpSduVolumeUL", name) == 0) {
    fprintf(file, "DRB.PdcpSduVolumeUL,%d,kb\n", meas_record.int_val);
  } else {
    fprintf(file, "Measurement Name not yet supported\n");
  }
}

static
void log_real_value(byte_array_t name, meas_record_lst_t meas_record, FILE *file)
{
  if (cmp_str_ba("DRB.RlcSduDelayDl", name) == 0) {
    fprintf(file, "DRB.RlcSduDelayDl,%.2f,us\n", meas_record.real_val);
  } else if (cmp_str_ba("DRB.UEThpDl", name) == 0) {
    fprintf(file, "DRB.UEThpDl,%.2f,kbps\n", meas_record.real_val);
  } else if (cmp_str_ba("DRB.UEThpUl", name) == 0) {
    fprintf(file, "DRB.UEThpUl,%.2f,kbps\n", meas_record.real_val);
  } else {
    fprintf(file, "Measurement Name not yet supported\n");
  }
}
typedef void (*log_meas_value)(byte_array_t name, meas_record_lst_t meas_record, FILE *file);

static
log_meas_value get_meas_value[END_MEAS_VALUE] = {
    log_int_value,
    log_real_value,
    NULL,
};

static
void match_meas_name_type(meas_type_t meas_type, meas_record_lst_t meas_record, FILE *file)
{
  // Get the value of the Measurement
  get_meas_value[meas_record.value](meas_type.name, meas_record, file);
}

static
void match_id_meas_type(meas_type_t meas_type, meas_record_lst_t meas_record, FILE *file)
{
  (void)file;  // Suppress unused parameter warning
  (void)meas_type;
  (void)meas_record;
  assert(false && "ID Measurement Type not yet supported");
}

typedef void (*check_meas_type)(meas_type_t meas_type, meas_record_lst_t meas_record, FILE *file);

static
check_meas_type match_meas_type[END_MEAS_TYPE] = {
    match_meas_name_type,
    match_id_meas_type,
};

static
void log_kpm_measurements(kpm_ind_msg_format_1_t const* msg_frm_1, FILE *file)
{
  assert(msg_frm_1->meas_info_lst_len > 0 && "Cannot correctly print measurements");

  // UE Measurements per granularity period
  for (size_t j = 0; j < msg_frm_1->meas_data_lst_len; j++) {
    meas_data_lst_t const data_item = msg_frm_1->meas_data_lst[j];

    for (size_t z = 0; z < data_item.meas_record_len; z++) {
      meas_type_t const meas_type = msg_frm_1->meas_info_lst[z].meas_type;
      meas_record_lst_t const record_item = data_item.meas_record_lst[z];

      match_meas_type[meas_type.type](meas_type, record_item, file);

      if (data_item.incomplete_flag && *data_item.incomplete_flag == TRUE_ENUM_VALUE)
        printf("Measurement Record not reliable");
    }
  }

}

static
void sm_cb_kpm(sm_ag_if_rd_t const* rd) //Setareh : here is the part of requesting for different data
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == KPM_STATS_V3_0);
  // Use the global file pointer to log data
  // if (file != NULL) {
  //     fprintf(file, "Logging some data...\n");}
      fflush(file);  // Ensure the data is flushed to the file immediately

  // Reading Indication Message Format 3
  kpm_ind_data_t const* ind = &rd->ind.kpm.ind;
  kpm_ric_ind_hdr_format_1_t const* hdr_frm_1 = &ind->hdr.kpm_ric_ind_hdr_format_1;
  kpm_ind_msg_format_3_t const* msg_frm_3 = &ind->msg.frm_3;

  int64_t const now = time_now_us();
  static int counter = 1;
  {
    lock_guard(&mtx);

    fprintf(file,"\n%7d KPM ind_msg latency = %ld [μs]\n", counter, now - hdr_frm_1->collectStartTime); // xApp <-> E2 Node
    fflush(file);  // Ensure the data is flushed to the file immediately

    // Reported list of measurements per UE
    for (size_t i = 0; i < msg_frm_3->ue_meas_report_lst_len; i++) {
      // log UE ID
      ue_id_e2sm_t const ue_id_e2sm = msg_frm_3->meas_report_per_ue[i].ue_meas_report_lst;
      ue_id_e2sm_e const type = ue_id_e2sm.type;
      log_ue_id_e2sm[type](ue_id_e2sm, file); // Setareh : here it log the info until ran_ue_id
      fflush(file);  // Ensure the data is flushed to the file immediately

      // log measurements
      log_kpm_measurements(&msg_frm_3->meas_report_per_ue[i].ind_msg_format_1, file); // // Setareh : here it log the measurment
      fflush(file);  // Ensure the data is flushed to the file immediately
      // fclose(file);  // Close the file when you're done

    }
    counter++;
  }
}

static
test_info_lst_t filter_predicate(test_cond_type_e type, test_cond_e cond, int value)
{
  test_info_lst_t dst = {0};

  dst.test_cond_type = type;
  // It can only be TRUE_TEST_COND_TYPE so it does not matter the type
  // but ugly ugly...
  dst.S_NSSAI = TRUE_TEST_COND_TYPE;

  dst.test_cond = calloc(1, sizeof(test_cond_e));
  assert(dst.test_cond != NULL && "Memory exhausted");
  *dst.test_cond = cond;

  dst.test_cond_value = calloc(1, sizeof(test_cond_value_t));
  assert(dst.test_cond_value != NULL && "Memory exhausted");
  dst.test_cond_value->type = OCTET_STRING_TEST_COND_VALUE;

  dst.test_cond_value->octet_string_value = calloc(1, sizeof(byte_array_t));
  assert(dst.test_cond_value->octet_string_value != NULL && "Memory exhausted");
  const size_t len_nssai = 4;  // it was 1, as only sst was filled
  dst.test_cond_value->octet_string_value->len = len_nssai;
  dst.test_cond_value->octet_string_value->buf = calloc(len_nssai, sizeof(uint8_t));
  assert(dst.test_cond_value->octet_string_value->buf != NULL && "Memory exhausted");
  dst.test_cond_value->octet_string_value->buf[0] = value;  // sst
  printf("SST value in filter_predicate %d",value);
  const uint32_t sd = value;  // hardcoded value to your specific case
  dst.test_cond_value->octet_string_value->buf[1] = sd >> 16;
  dst.test_cond_value->octet_string_value->buf[2] = sd >> 8;
  dst.test_cond_value->octet_string_value->buf[3] = sd;

  return dst;
}

static
label_info_lst_t fill_kpm_label(void)
{
  label_info_lst_t label_item = {0};

  label_item.noLabel = ecalloc(1, sizeof(enum_value_e));
  *label_item.noLabel = TRUE_ENUM_VALUE;

  return label_item;
}

static
kpm_act_def_format_1_t fill_act_def_frm_1(ric_report_style_item_t const* report_item, int* matched_kpis, int matched_kpi_count, uint64_t period_ms)
{
  assert(report_item != NULL);

  kpm_act_def_format_1_t ad_frm_1 = {0};

  // size_t const sz = report_item->meas_info_for_action_lst_len;
  size_t const sz = matched_kpi_count;

  // [1, 65535]
  ad_frm_1.meas_info_lst_len = sz;
  ad_frm_1.meas_info_lst = calloc(sz, sizeof(meas_info_format_1_lst_t));
  assert(ad_frm_1.meas_info_lst != NULL && "Memory exhausted");

  for (size_t i = 0; i < sz; i++) {
    meas_info_format_1_lst_t* meas_item = &ad_frm_1.meas_info_lst[i];
    // 8.3.9
    // Measurement Name
    meas_item->meas_type.type = NAME_MEAS_TYPE;
    meas_item->meas_type.name = copy_byte_array(report_item->meas_info_for_action_lst[matched_kpis[i]].name);

    // [1, 2147483647]
    // 8.3.11
    meas_item->label_info_lst_len = 1;
    meas_item->label_info_lst = ecalloc(1, sizeof(label_info_lst_t));
    meas_item->label_info_lst[0] = fill_kpm_label();
  }

  // 8.3.8 [0, 4294967295]
  ad_frm_1.gran_period_ms = period_ms;

  // 8.3.20 - OPTIONAL
  ad_frm_1.cell_global_id = NULL;

#if defined KPM_V2_03 || defined KPM_V3_00
  // [0, 65535]
  ad_frm_1.meas_bin_range_info_lst_len = 0;
  ad_frm_1.meas_bin_info_lst = NULL;
#endif

  return ad_frm_1;
}

static
kpm_act_def_t fill_report_style_4(ric_report_style_item_t const* report_item, int* matched_kpis, int matched_kpi_count, uint64_t period_ms)
{
  kpm_act_def_t act_def = {.type = FORMAT_4_ACTION_DEFINITION};

  // Fill matching condition
  // [1, 32768]
  act_def.frm_4.matching_cond_lst_len = 1;
  act_def.frm_4.matching_cond_lst = calloc(act_def.frm_4.matching_cond_lst_len, sizeof(matching_condition_format_4_lst_t));
  assert(act_def.frm_4.matching_cond_lst != NULL && "Memory exhausted");
  // Filter connected UEs by S-NSSAI criteria
  test_cond_type_e const type = S_NSSAI_TEST_COND_TYPE; // CQI_TEST_COND_TYPE
  test_cond_e const condition = EQUAL_TEST_COND; // GREATERTHAN_TEST_COND
  // int const value = 1;
  // Use the 'value' from report_item->value
  int value = report_item->value; 
  act_def.frm_4.matching_cond_lst[0].test_info_lst = filter_predicate(type, condition, value);

  // Fill Action Definition Format 1
  // 8.2.1.2.1
  act_def.frm_4.action_def_format_1 = fill_act_def_frm_1(report_item, matched_kpis, matched_kpi_count, period_ms);

  return act_def;
}


typedef kpm_act_def_t (*fill_kpm_act_def)(ric_report_style_item_t const* report_item);


static
kpm_sub_data_t gen_kpm_subs(kpm_ran_function_def_t const* ran_func, int value)
{
  assert(ran_func != NULL);
  assert(ran_func->ric_event_trigger_style_list != NULL);

  kpm_sub_data_t kpm_sub = {0};

  // Parse KPM config file
  uint64_t period_ms = 0;
  char **kpi_list = {0};
  int kpi_count = 0;
  parse_yaml_file(&period_ms, &kpi_list, &kpi_count);
  printf("Read KPM config file: \n");
  printf("Granularity: %lu ms\n", period_ms);
  printf("Requested KPIs: \n");
  for (int i = 0; i < kpi_count; i++) {
    printf("- %s\n", kpi_list[i]);
  }

  // Generate Event Trigger
  assert(ran_func->ric_event_trigger_style_list[0].format_type == FORMAT_1_RIC_EVENT_TRIGGER);
  kpm_sub.ev_trg_def.type = FORMAT_1_RIC_EVENT_TRIGGER;
  kpm_sub.ev_trg_def.kpm_ric_event_trigger_format_1.report_period_ms = period_ms;

  // Validate requested KPIs with the available KPIs, finding the index of the requested KPI in the report_item
  ric_report_style_item_t* const report_item = &ran_func->ric_report_style_list[0];
  report_item->value = value; // You can dynamically assign this based on some condition
  // ric_service_report_e const report_style_type = report_item->report_style_type;
  // printf("fill_report_style: %d\n", report_style_type);

  size_t const sz = report_item->meas_info_for_action_lst_len;
  int matched_kpis[sz];
  int matched_kpi_count = 0;
  for (int i = 0; i < kpi_count; i++) {
    for (int j = 0; j < sz; j++) {
      if (strcmp(kpi_list[i], (char *)report_item->meas_info_for_action_lst[j].name.buf) == 0) {
        // printf("Matched KPI: %s\n", kpi_list[i]);
        matched_kpis[matched_kpi_count++] = j;
        break;
      }
      if (j == sz - 1) {
        printf("KPI not found: %s, skipping!\n", kpi_list[i]);
      }
    }
  }
  printf("\n");

  // Generate Action Definition
  kpm_sub.sz_ad = 1;
  kpm_sub.ad = calloc(kpm_sub.sz_ad, sizeof(kpm_act_def_t));
  assert(kpm_sub.ad != NULL && "Memory exhausted");
  assert(report_item != NULL);
  assert(report_item->act_def_format_type == FORMAT_4_ACTION_DEFINITION);
  *kpm_sub.ad = fill_report_style_4(report_item, matched_kpis, matched_kpi_count, period_ms);

  return kpm_sub;
}

static
bool eq_sm(sm_ran_function_t const* elem, int const id)
{
  if (elem->id == id)
    return true;

  return false;
}

static
size_t find_sm_idx(sm_ran_function_t* rf, size_t sz, bool (*f)(sm_ran_function_t const*, int const), int const id)
{
  for (size_t i = 0; i < sz; i++) {
    if (f(&rf[i], id))
      return i;
  }

  assert(0 != 0 && "SM ID could not be found in the RAN Function List");
}

int main(int argc, char* argv[])
{
  fr_args_t args = init_fr_args(argc, argv);

  // Init the xApp
  init_xapp_api(&args);
  sleep(1);
  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });
  assert(nodes.len > 0);
  printf("Connected E2 nodes = %d\n", nodes.len);

  pthread_mutexattr_t attr = {0};
  int rc = pthread_mutex_init(&mtx, &attr);
  assert(rc == 0);

  sm_ans_xapp_t* hndl = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  assert(hndl != NULL);

  ////////////
  // START KPM
  ////////////
  int const KPM_ran_function = 2;
  e2_node_connected_xapp_t* n = &nodes.n[0];
  size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, KPM_ran_function);
  assert(n->rf[idx].defn.type == KPM_RAN_FUNC_DEF_E && "KPM is not the received RAN Function");

  // if REPORT Service is supported by E2 node, send SUBSCRIPTION
  // e.g. OAI CU-CP
  if (n->rf[idx].defn.kpm.ric_report_style_list != NULL) {
    file = fopen("/home/user/workshop/collected_metrics/kpm_measurements.log", "w");  // Open the file in write mode
    if (file == NULL) {
      perror("Failed to open file");
      return 1;
    }

    // Generate KPM SUBSCRIPTION message
    int value = 1;
    kpm_sub_data_t kpm_sub = gen_kpm_subs(&n->rf[idx].defn.kpm,value);

    hndl[0] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
    assert(hndl[0].success == true);

    // Wait for the user to stop the xApp
    getchar();

    // END KPM
    free_kpm_sub_data(&kpm_sub);

    fclose(file);  // Close the file when you're done
  }

  // Remove the handle previously returned
  if (hndl[0].success == true)
    rm_report_sm_xapp_api(hndl[0].u.handle); //this is the part we can comment for having the report in cycle
  free(hndl);

  // Stop the xApp
  while (try_stop_xapp_api() == false)
    usleep(1000);

  printf("xApp terminated SUCCESSFULLY\n");
}
