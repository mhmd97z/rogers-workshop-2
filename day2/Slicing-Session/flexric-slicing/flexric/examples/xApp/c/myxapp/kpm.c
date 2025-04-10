#include "kpm.h"
#include <assert.h>
#include <stdio.h>
#include <cjson/cJSON.h>

uint64_t const period_ms_kpm_S = 1000;
pthread_mutex_t mtx;
ue_id_e2sm_t ue = {0};
// FIXME: This is a global variable, which is not recommended, use static globalvariable instead
// FILE *file = NULL;  // Global definition
// FILE *csv_file = NULL; 
extern const char* monitored_csv;
const char* monitored_json = "/home/user/workshop/collected_metrics/exporter_stats_kpm.json";

extern const char* ue_info_csv;
static int kpm_counter = 1;
// void store_ue_id_to_csv(ue_id_e2sm_t *ue_id, FILE *csv_file) {
//     if (ue_id->type != GNB_UE_ID_E2SM) {
//         fprintf(stderr, "Unsupported UE ID type\n");
//         return;
//     }

//     // Write the data to the CSV file
//     fprintf(csv_file, "%lu,%u,%u,%u,%u,%u,%u\n",
//             ue_id->gnb.amf_ue_ngap_id,
//             ue_id->gnb.guami.plmn_id.mcc,
//             ue_id->gnb.guami.plmn_id.mnc,
//             ue_id->gnb.guami.plmn_id.mnc_digit_len,
//             ue_id->gnb.guami.amf_region_id,
//             ue_id->gnb.guami.amf_set_id,
//             ue_id->gnb.guami.amf_ptr);
//     fflush(csv_file);
// }

// Function to compare two entries
bool is_duplicate(const ue_entry_t *a, const ue_entry_t *b) {
    return (a->amf_ue_ngap_id == b->amf_ue_ngap_id &&
            a->mcc == b->mcc &&
            a->mnc == b->mnc &&
            a->mnc_digit_len == b->mnc_digit_len &&
            a->amf_region_id == b->amf_region_id &&
            a->amf_set_id == b->amf_set_id &&
            a->amf_ptr == b->amf_ptr);
}

// Function to load existing entries from the CSV file
size_t load_existing_entries(const char* filename, ue_entry_t **entries) {
    FILE *file = fopen(filename, "r");
    assert(file != NULL && "Error opening the CSV file");

    char line[256];
    size_t count = 0;
    *entries = NULL;

    // Skip the header
    fgets(line, sizeof(line), file);

    // Read lines into memory
    while (fgets(line, sizeof(line), file)) {
        ue_entry_t entry = {0};
        sscanf(line, "%lu,%u,%u,%u,%u,%u,%u",
               &entry.amf_ue_ngap_id,
               &entry.mcc,
               &entry.mnc,
               &entry.mnc_digit_len,
               &entry.amf_region_id,
               &entry.amf_set_id,
               &entry.amf_ptr);

        // Reallocate memory to store the new entry
        *entries = realloc(*entries, sizeof(ue_entry_t) * (count + 1));
        (*entries)[count++] = entry;
    }

    fclose(file);
    return count;
}

// Function to store UE ID if not duplicate
void store_ue_id_to_csv_if_unique(const ue_id_e2sm_t *ue_id, const ue_entry_t *existing_entries, size_t existing_count) {
    ue_entry_t new_entry = {
        .amf_ue_ngap_id = ue_id->gnb.amf_ue_ngap_id,
        .mcc = ue_id->gnb.guami.plmn_id.mcc,
        .mnc = ue_id->gnb.guami.plmn_id.mnc,
        .mnc_digit_len = ue_id->gnb.guami.plmn_id.mnc_digit_len,
        .amf_region_id = ue_id->gnb.guami.amf_region_id,
        .amf_set_id = ue_id->gnb.guami.amf_set_id,
        .amf_ptr = ue_id->gnb.guami.amf_ptr,
    };

    // Check if the entry already exists
    for (size_t i = 0; i < existing_count; i++) {
        if (is_duplicate(&new_entry, &existing_entries[i])) {
            // Duplicate found, don't write
            return;
        }
    }
    // open the file
    FILE *csv_file = fopen(ue_info_csv, "a");
    assert(csv_file != NULL && "Error opening the CSV file");

    // Write the new entry to the CSV file
    fprintf(csv_file, "%lu,%u,%u,%u,%u,%u,%u\n",
            new_entry.amf_ue_ngap_id,
            new_entry.mcc,
            new_entry.mnc,
            new_entry.mnc_digit_len,
            new_entry.amf_region_id,
            new_entry.amf_set_id,
            new_entry.amf_ptr);
    fflush(csv_file);
    fclose(csv_file);

}

bool eq_sm(sm_ran_function_t const* elem, int const id)
{
  if (elem->id == id)
    return true;

  return false;
}


size_t find_sm_idx(sm_ran_function_t* rf, size_t sz, bool (*f)(sm_ran_function_t const*, int const), int const id)
{
  for (size_t i = 0; i < sz; i++) {
    if (f(&rf[i], id))
      return i;
  }

  assert(0 != 0 && "SM ID could not be found in the RAN Function List");
}



kpm_sub_data_t gen_kpm_subs(kpm_ran_function_def_t const* ran_func,int value)
{
  assert(ran_func != NULL);
  assert(ran_func->ric_event_trigger_style_list != NULL);

  kpm_sub_data_t kpm_sub = {0};

  // Generate Event Trigger
  assert(ran_func->ric_event_trigger_style_list[0].format_type == FORMAT_1_RIC_EVENT_TRIGGER);
  kpm_sub.ev_trg_def.type = FORMAT_1_RIC_EVENT_TRIGGER;
  kpm_sub.ev_trg_def.kpm_ric_event_trigger_format_1.report_period_ms = period_ms_kpm_S;

  // Generate Action Definition
  kpm_sub.sz_ad = 1;
  kpm_sub.ad = calloc(kpm_sub.sz_ad, sizeof(kpm_act_def_t));
  assert(kpm_sub.ad != NULL && "Memory exhausted");

  // Multiple Action Definitions in one SUBSCRIPTION message is not supported in this project
  // Multiple REPORT Styles = Multiple Action Definition = Multiple SUBSCRIPTION messages
  ric_report_style_item_t* const report_item = &ran_func->ric_report_style_list[0];
  report_item->value = value; // You can dynamically assign this based on some condition
  ric_service_report_e const report_style_type = report_item->report_style_type;
  *kpm_sub.ad = get_kpm_act_def_S[report_style_type](report_item);

  return kpm_sub;
}


kpm_act_def_t fill_report_style_4(ric_report_style_item_t const* report_item)
{
  assert(report_item != NULL);
  assert(report_item->act_def_format_type == FORMAT_4_ACTION_DEFINITION);

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
  act_def.frm_4.action_def_format_1 = fill_act_def_frm_1(report_item);

  return act_def;
}


kpm_act_def_format_1_t fill_act_def_frm_1(ric_report_style_item_t const* report_item)
{
  assert(report_item != NULL);

  kpm_act_def_format_1_t ad_frm_1 = {0};

  size_t const sz = report_item->meas_info_for_action_lst_len;

  // [1, 65535]
  ad_frm_1.meas_info_lst_len = sz;
  ad_frm_1.meas_info_lst = calloc(sz, sizeof(meas_info_format_1_lst_t));
  assert(ad_frm_1.meas_info_lst != NULL && "Memory exhausted");

  for (size_t i = 0; i < sz; i++) {
    meas_info_format_1_lst_t* meas_item = &ad_frm_1.meas_info_lst[i];
    // 8.3.9
    // Measurement Name
    meas_item->meas_type.type = NAME_MEAS_TYPE;
    meas_item->meas_type.name = copy_byte_array(report_item->meas_info_for_action_lst[i].name);

    // [1, 2147483647]
    // 8.3.11
    meas_item->label_info_lst_len = 1;
    meas_item->label_info_lst = ecalloc(1, sizeof(label_info_lst_t));
    meas_item->label_info_lst[0] = fill_kpm_label();
  }

  // 8.3.8 [0, 4294967295]
  ad_frm_1.gran_period_ms = period_ms_kpm_S;

  // 8.3.20 - OPTIONAL
  ad_frm_1.cell_global_id = NULL;

#if defined KPM_V2_03 || defined KPM_V3_00
  // [0, 65535]
  ad_frm_1.meas_bin_range_info_lst_len = 0;
  ad_frm_1.meas_bin_info_lst = NULL;
#endif

  return ad_frm_1;
}

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

  // dst.test_cond_value->octet_string_value = calloc(1, sizeof(byte_array_t));
  // assert(dst.test_cond_value->octet_string_value != NULL && "Memory exhausted");
  // const size_t len_nssai = 1;
  // dst.test_cond_value->octet_string_value->len = len_nssai;
  // dst.test_cond_value->octet_string_value->buf = calloc(len_nssai, sizeof(uint8_t));
  // assert(dst.test_cond_value->octet_string_value->buf != NULL && "Memory exhausted");
  // dst.test_cond_value->octet_string_value->buf[0] = value;
  

  dst.test_cond_value->octet_string_value = calloc(1, sizeof(byte_array_t));
  assert(dst.test_cond_value->octet_string_value != NULL && "Memory exhausted");
  const size_t len_nssai = 4;  // it was 1, as only sst was filled
  dst.test_cond_value->octet_string_value->len = len_nssai;
  dst.test_cond_value->octet_string_value->buf = calloc(len_nssai, sizeof(uint8_t));
  assert(dst.test_cond_value->octet_string_value->buf != NULL && "Memory exhausted");
  dst.test_cond_value->octet_string_value->buf[0] = value;  // sst
  printf("SST value in filter_predicate %d\n",value);
  const uint32_t sd = value;  // hardcoded value to your specific case
  dst.test_cond_value->octet_string_value->buf[1] = sd >> 16;
  dst.test_cond_value->octet_string_value->buf[2] = sd >> 8;
  dst.test_cond_value->octet_string_value->buf[3] = sd;

  return dst;
}


label_info_lst_t fill_kpm_label(void)
{
  label_info_lst_t label_item = {0};

  label_item.noLabel = ecalloc(1, sizeof(enum_value_e));
  *label_item.noLabel = TRUE_ENUM_VALUE;

  return label_item;
}

// Function to save data in Json
void write_json(const char *json_file, kpm_entry_t *kpm_entry, size_t length) {
    FILE *file = fopen(json_file, "w");  // Open in write mode to replace the file
    if (!file) {
        perror("Failed to open file");
        return;
    }

    cJSON *json_root = cJSON_CreateObject();

    for (size_t i = 0; i < length; i++) {
        char key[32];  
        snprintf(key, sizeof(key), "%lu", kpm_entry[i].ran_ue_id); // Use ran_ue_id as key

        cJSON *kpm_data = cJSON_CreateObject();
        
        cJSON_AddNumberToObject(kpm_data, "latency", kpm_entry[i].latency);
        cJSON_AddNumberToObject(kpm_data, "ue_id_type", kpm_entry[i].ue_id_type);
        cJSON_AddNumberToObject(kpm_data, "amf_ue_ngap_id", kpm_entry[i].amf_ue_ngap_id);
        cJSON_AddNumberToObject(kpm_data, "ran_ue_id", kpm_entry[i].ran_ue_id);
        cJSON_AddNumberToObject(kpm_data, "pdcp_sdu_volume_dl", kpm_entry[i].pdcp_sdu_volume_dl);
        cJSON_AddNumberToObject(kpm_data, "pdcp_sdu_volume_ul", kpm_entry[i].pdcp_sdu_volume_ul);
        cJSON_AddNumberToObject(kpm_data, "rlc_sdu_delay_dl", kpm_entry[i].rlc_sdu_delay_dl);
        cJSON_AddNumberToObject(kpm_data, "ue_thp_dl", kpm_entry[i].ue_thp_dl);
        cJSON_AddNumberToObject(kpm_data, "ue_thp_ul", kpm_entry[i].ue_thp_ul);
        cJSON_AddNumberToObject(kpm_data, "prb_tot_dl", kpm_entry[i].prb_tot_dl);
        cJSON_AddNumberToObject(kpm_data, "prb_tot_ul", kpm_entry[i].prb_tot_ul);
        cJSON_AddNumberToObject(kpm_data, "slice_id", kpm_entry[i].slice_id);

        cJSON *mac_object = cJSON_CreateObject();
        cJSON_AddItemToObject(mac_object, "kpm", kpm_data);

        cJSON_AddItemToObject(json_root, key, mac_object);
    }

    char *json_string = cJSON_Print(json_root);  // Convert to formatted JSON string
    // printf("Generated JSON Data:\n%s\n", json_string);

    fprintf(file, "%s", json_string);
    
    fclose(file);
    cJSON_Delete(json_root);
    free(json_string);
}

void sm_cb_kpm(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == KPM_STATS_V3_0);


  // Reading Indication Message Format 3
  kpm_ind_data_t const* ind = &rd->ind.kpm.ind;
  kpm_ric_ind_hdr_format_1_t const* hdr_frm_1 = &ind->hdr.kpm_ric_ind_hdr_format_1;
  kpm_ind_msg_format_3_t const* msg_frm_3 = &ind->msg.frm_3;

  int64_t const now = time_now_us();

  lock_guard(&mtx);
  assert(msg_frm_3->ue_meas_report_lst_len > 0 && "No UE Measurement Report received, There is no UE.");

  kpm_entry_t* kpm_entry = calloc(msg_frm_3->ue_meas_report_lst_len, sizeof(kpm_entry_t));
  assert(kpm_entry != NULL && "Memory exhausted");


  // FILE *csv_file = fopen("/root/flexric-slicing/ue_data.csv", "a");
  // // Write CSV header if the file is empty
  // if (ftell(csv_file) == 0) {
  //     fprintf(csv_file, "AMF_UE_NGAP_ID,MCC,MNC,MNC_DIGIT_LEN,AMF_REGION_ID,AMF_SET_ID,AMF_PTR\n");
  // }

  // Reported list of measurements per UE
  for (size_t i = 0; i < msg_frm_3->ue_meas_report_lst_len; i++) {
    // log UE ID
    kpm_entry[i].latency = now - (int64_t)hdr_frm_1->collectStartTime;
    printf("\n%7d KPM ind_msg latency = %ld [μs]\n", kpm_counter, kpm_entry[i].latency); // xApp <-> E2 Node
    ue_id_e2sm_t const ue_id_e2sm = msg_frm_3->meas_report_per_ue[i].ue_meas_report_lst;
    ue_id_e2sm_e const type = ue_id_e2sm.type;

    ue = ue_id_e2sm;
    // Generate and store a UE ID
    // store_ue_id_to_csv(&ue, csv_file);
    // Load existing entries
    ue_entry_t *existing_entries = NULL;
    size_t existing_count = load_existing_entries(ue_info_csv, &existing_entries);

    // Generate and store a UE ID
    store_ue_id_to_csv_if_unique(&ue, existing_entries, existing_count);

    // Free memory for existing entries
    free(existing_entries);

    // fprintf(file,"Here is UE amf_ue_ngap_id: %lu \n",ue.gnb.amf_ue_ngap_id);
    // fflush(file);

    log_ue_id_e2sm_S[type](ue_id_e2sm, &kpm_entry[i]);
    // log_ue_id_e2sm_S[type](ue_id_e2sm);

    // log measurements
    // log_kpm_measurements(&msg_frm_3->meas_report_per_ue[i].ind_msg_format_1);
    log_kpm_measurements(&msg_frm_3->meas_report_per_ue[i].ind_msg_format_1,&kpm_entry[i]);
  }

  FILE *file = fopen(monitored_csv, "a");
  for (size_t i = 0; i < msg_frm_3->ue_meas_report_lst_len; i++) {
    fprintf(file, "%ld,%d,%lu,%lx,%d,%d,%f,%f,%f,%d,%d,%d\n",
            kpm_entry[i].latency,
            kpm_entry[i].ue_id_type,
            kpm_entry[i].amf_ue_ngap_id,
            kpm_entry[i].ran_ue_id,
            kpm_entry[i].pdcp_sdu_volume_dl,
            kpm_entry[i].pdcp_sdu_volume_ul,
            kpm_entry[i].rlc_sdu_delay_dl,
            kpm_entry[i].ue_thp_dl,
            kpm_entry[i].ue_thp_ul,
            kpm_entry[i].prb_tot_dl,
            kpm_entry[i].prb_tot_ul,
            kpm_entry[i].slice_id);
  }
  fclose(file);

  write_json(monitored_json, kpm_entry, msg_frm_3->ue_meas_report_lst_len);

  free(kpm_entry);
  kpm_counter++;
}


// void log_gnb_ue_id(ue_id_e2sm_t ue_id)
// {
//   if (ue_id.gnb.gnb_cu_ue_f1ap_lst != NULL) {
//     for (size_t i = 0; i < ue_id.gnb.gnb_cu_ue_f1ap_lst_len; i++) {
//       printf("UE ID type = gNB-CU, gnb_cu_ue_f1ap = %u\n", ue_id.gnb.gnb_cu_ue_f1ap_lst[i]);
//     }
//   } else {
//     printf("UE ID type = gNB, amf_ue_ngap_id = %lu\n", ue_id.gnb.amf_ue_ngap_id);
//   }
//   if (ue_id.gnb.ran_ue_id != NULL) {
//     printf("ran_ue_id = %lx\n", *ue_id.gnb.ran_ue_id); // RAN UE NGAP ID
//   }
// }

void log_gnb_ue_id(ue_id_e2sm_t ue_id, kpm_entry_t* kpm_entry)
{
  if (ue_id.gnb.gnb_cu_ue_f1ap_lst != NULL) {
    for (size_t i = 0; i < ue_id.gnb.gnb_cu_ue_f1ap_lst_len; i++) {
      printf("UE ID type,gNB-CU,gnb_cu_ue_f1ap,%u\n", ue_id.gnb.gnb_cu_ue_f1ap_lst[i]);
      kpm_entry->ue_id_type = ue_id.gnb.gnb_cu_ue_f1ap_lst[i];

    }
  } else {
    printf("UE ID type,gNB,amf_ue_ngap_id,%lu\n", ue_id.gnb.amf_ue_ngap_id);
    kpm_entry->amf_ue_ngap_id = ue_id.gnb.amf_ue_ngap_id;
  }

  if (ue_id.gnb.ran_ue_id != NULL) {
    printf("ran_ue_id,%lx\n", *(ue_id.gnb.ran_ue_id));
    kpm_entry->ran_ue_id = *(ue_id.gnb.ran_ue_id);
  }
}

// void log_du_ue_id(ue_id_e2sm_t ue_id)
// {
//   printf("UE ID type = gNB-DU, gnb_cu_ue_f1ap = %u\n", ue_id.gnb_du.gnb_cu_ue_f1ap);
//   if (ue_id.gnb_du.ran_ue_id != NULL) {
//     printf("ran_ue_id = %lx\n", *ue_id.gnb_du.ran_ue_id); // RAN UE NGAP ID
//   }
// }

void log_du_ue_id(ue_id_e2sm_t ue_id, kpm_entry_t* kpm_entry)
{
  (void)kpm_entry;  // Suppress unused parameter warning
  printf("UE ID type = gNB-DU, gnb_cu_ue_f1ap = %u\n", ue_id.gnb_du.gnb_cu_ue_f1ap);
  if (ue_id.gnb_du.ran_ue_id != NULL) {
    printf("ran_ue_id = %lx\n", *ue_id.gnb_du.ran_ue_id); // RAN UE NGAP ID 
  }
}

// void log_cuup_ue_id(ue_id_e2sm_t ue_id)
// {
//   printf("UE ID type = gNB-CU-UP, gnb_cu_cp_ue_e1ap = %u\n", ue_id.gnb_cu_up.gnb_cu_cp_ue_e1ap);
//   if (ue_id.gnb_cu_up.ran_ue_id != NULL) {
//     printf("ran_ue_id = %lx\n", *ue_id.gnb_cu_up.ran_ue_id); // RAN UE NGAP ID
//   }
// }
void log_cuup_ue_id(ue_id_e2sm_t ue_id, kpm_entry_t* kpm_entry)
{
  (void)kpm_entry;  // Suppress unused parameter warning
  printf("UE ID type = gNB-CU-UP, gnb_cu_cp_ue_e1ap = %u\n", ue_id.gnb_cu_up.gnb_cu_cp_ue_e1ap);
  if (ue_id.gnb_cu_up.ran_ue_id != NULL) {
    printf("ran_ue_id = %lx\n", *ue_id.gnb_cu_up.ran_ue_id); // RAN UE NGAP ID
  }
}

void log_int_value(byte_array_t name, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry)
{
  if (cmp_str_ba("RRU.PrbTotDl", name) == 0) {
    printf("RRU.PrbTotDl = %d [PRBs]\n", meas_record.int_val);
    kpm_entry->prb_tot_dl = meas_record.int_val;
  } else if (cmp_str_ba("RRU.PrbTotUl", name) == 0) {
    printf("RRU.PrbTotUl = %d [PRBs]\n", meas_record.int_val);
    kpm_entry->prb_tot_ul = meas_record.int_val;
  } else if (cmp_str_ba("DRB.PdcpSduVolumeDL", name) == 0) {
    printf("DRB.PdcpSduVolumeDL = %d [kb]\n", meas_record.int_val);
    kpm_entry->pdcp_sdu_volume_dl = meas_record.int_val;
  } else if (cmp_str_ba("DRB.PdcpSduVolumeUL", name) == 0) {
    printf("DRB.PdcpSduVolumeUL = %d [kb]\n", meas_record.int_val);
    kpm_entry->pdcp_sdu_volume_ul = meas_record.int_val;
  } else if (cmp_str_ba("Slice_ID", name) == 0) {
    printf("Slice_ID = %d\n", meas_record.int_val);
    kpm_entry->slice_id = meas_record.int_val;
  } else {
    printf("Measurement Name not yet supported\n");
  }
}

// void log_int_value(byte_array_t name, meas_record_lst_t meas_record)
// {
//   if (cmp_str_ba("RRU.PrbTotDl", name) == 0) {
//     printf("RRU.PrbTotDl = %d [PRBs]\n", meas_record.int_val);
//   } else if (cmp_str_ba("RRU.PrbTotUl", name) == 0) {
//     printf("RRU.PrbTotUl = %d [PRBs]\n", meas_record.int_val);
//   } else if (cmp_str_ba("DRB.PdcpSduVolumeDL", name) == 0) {
//     printf("DRB.PdcpSduVolumeDL = %d [kb]\n", meas_record.int_val);
//   } else if (cmp_str_ba("DRB.PdcpSduVolumeUL", name) == 0) {
//     printf("DRB.PdcpSduVolumeUL = %d [kb]\n", meas_record.int_val);
//   } else {
//     printf("Measurement Name not yet supported\n");
//   }
// }

// void log_real_value(byte_array_t name, meas_record_lst_t meas_record)
// {
//   if (cmp_str_ba("DRB.RlcSduDelayDl", name) == 0) {
//     fprintf("DRB.RlcSduDelayDl = %.2f [μs]\n", meas_record.real_val);
//   } else if (cmp_str_ba("DRB.UEThpDl", name) == 0) {
//     fprintf("DRB.UEThpDl = %.2f [kbps]\n", meas_record.real_val);
//   } else if (cmp_str_ba("DRB.UEThpUl", name) == 0) {
//     fprintf("DRB.UEThpUl = %.2f [kbps]\n", meas_record.real_val);
//   } else {
//     fprintf("Measurement Name not yet supported\n");
//   }
// }


void log_real_value(byte_array_t name, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry)
{
  if (cmp_str_ba("DRB.RlcSduDelayDl", name) == 0) {
    printf("DRB.RlcSduDelayDl,%.2f,us\n", meas_record.real_val);
    kpm_entry->rlc_sdu_delay_dl = meas_record.real_val;
  } else if (cmp_str_ba("DRB.UEThpDl", name) == 0) {
    printf("DRB.UEThpDl,%.2f,kbps\n", meas_record.real_val);
    kpm_entry->ue_thp_dl = meas_record.real_val;
  } else if (cmp_str_ba("DRB.UEThpUl", name) == 0) {
    printf("DRB.UEThpUl,%.2f,kbps\n", meas_record.real_val);
    kpm_entry->ue_thp_ul = meas_record.real_val;
  } else {
    printf("Measurement Name not yet supported\n");
  }
}
typedef void (*log_meas_value)(byte_array_t name, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry);

// void match_meas_name_type(meas_type_t meas_type, meas_record_lst_t meas_record)
// {
//   // Get the value of the Measurement
//   get_meas_value_S[meas_record.value](meas_type.name, meas_record);
// }
void match_meas_name_type(meas_type_t meas_type, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry)
{
  // Get the value of the Measurement
  get_meas_value_S[meas_record.value](meas_type.name, meas_record, kpm_entry);
}

// void match_id_meas_type(meas_type_t meas_type, meas_record_lst_t meas_record)
// {
//   (void)meas_type;
//   (void)meas_record;
//   assert(false && "ID Measurement Type not yet supported");
// }

void match_id_meas_type(meas_type_t meas_type, meas_record_lst_t meas_record, kpm_entry_t* kpm_entry)
{
  (void)kpm_entry;
  (void)meas_type;
  (void)meas_record;
  assert(false && "ID Measurement Type not yet supported");
}

void log_kpm_measurements(kpm_ind_msg_format_1_t const* msg_frm_1, kpm_entry_t* kpm_entry)
{
  assert(msg_frm_1->meas_info_lst_len > 0 && "Cannot correctly print measurements");
  assert(kpm_entry != NULL && "kpm_entry is NULL");

  // UE Measurements per granularity period
  for (size_t j = 0; j < msg_frm_1->meas_data_lst_len; j++) {
    meas_data_lst_t const data_item = msg_frm_1->meas_data_lst[j];

    for (size_t z = 0; z < data_item.meas_record_len; z++) {
      meas_type_t const meas_type = msg_frm_1->meas_info_lst[z].meas_type;
      meas_record_lst_t const record_item = data_item.meas_record_lst[z];

      match_meas_type_S[meas_type.type](meas_type, record_item, kpm_entry);
      // match_meas_type_S[meas_type.type](meas_type, record_item);

      if (data_item.incomplete_flag && *data_item.incomplete_flag == TRUE_ENUM_VALUE)
        printf("Measurement Record not reliable");
    }
  }

}

log_ue_id log_ue_id_e2sm_S[END_UE_ID_E2SM] = {
    log_gnb_ue_id, // common for gNB-mono, CU and CU-CP
    log_du_ue_id,
    log_cuup_ue_id,
    NULL,
    NULL,
    NULL,
    NULL,
};


log_meas_value get_meas_value_S[END_MEAS_VALUE] = {
    log_int_value,
    log_real_value,
    NULL,
};
check_meas_type match_meas_type_S[END_MEAS_TYPE] = {
    match_meas_name_type,
    match_id_meas_type,
};

fill_kpm_act_def get_kpm_act_def_S[END_RIC_SERVICE_REPORT] = {
    NULL,
    NULL,
    NULL,
    fill_report_style_4,
    NULL,
};