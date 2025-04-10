#include "rc_ho_slice.h"
#include "rc_slice.h"



e2sm_rc_ctrl_msg_t gen_rc_ctrl_HO_slice_level_msg(e2sm_rc_ctrl_msg_e msg_frmt, const char* sst_str, const char* sd_str)
{
  e2sm_rc_ctrl_msg_t dst = {0};

  if (msg_frmt == FORMAT_1_E2SM_RC_CTRL_MSG) {
    dst.format = msg_frmt;
    dst.frmt_1 = gen_rc_ctrl_msg_frmt_1_HO_slice_level(sst_str, sd_str);
  } else {
    assert(0!=0 && "not implemented the fill func for this ctrl msg frmt");
  }

  return dst;
}

void gen_HO_slice_level(seq_ran_param_t* HO_slice_level_msg, const char* sst_str, const char* sd_str)
{
  // The structure of HO slice lever msg: { HO_INFO{ PLMN, NSSAI{ SST, SD}}}
  HO_slice_level_msg->ran_param_id = HO_INFO;
  HO_slice_level_msg->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  HO_slice_level_msg->ran_param_val.strct = calloc(1, sizeof(seq_ran_param_t));
  assert(HO_slice_level_msg->ran_param_val.strct != NULL && "Memory exhausted");
  HO_slice_level_msg->ran_param_val.strct->sz_ran_param_struct = 2; // The 5 Paramaeters should be set in this list. (NSSAI(SST, SD), PLMN)
  HO_slice_level_msg->ran_param_val.strct->ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  assert(HO_slice_level_msg->ran_param_val.strct->ran_param_struct != NULL && "Memory exhausted");

  seq_ran_param_t* PLMN_Identity = &HO_slice_level_msg->ran_param_val.strct->ran_param_struct[0];
  PLMN_Identity->ran_param_id = PLMN_Identity_TEMP;
  PLMN_Identity->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  PLMN_Identity->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(PLMN_Identity->ran_param_val.flag_false != NULL && "Memory exhausted");
  PLMN_Identity->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  char plmnid_str[] = "00101";
  byte_array_t plmn_id = cp_str_to_ba(plmnid_str); // TODO
  PLMN_Identity->ran_param_val.flag_false->octet_str_ran.len = plmn_id.len;
  PLMN_Identity->ran_param_val.flag_false->octet_str_ran.buf = plmn_id.buf;

  // NSSAI
  seq_ran_param_t* DEST_NSSAI = &HO_slice_level_msg->ran_param_val.strct->ran_param_struct[1];
  DEST_NSSAI->ran_param_id = DEST_NSSAI_TEMP;
  DEST_NSSAI->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  DEST_NSSAI->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(DEST_NSSAI->ran_param_val.strct != NULL && "Memory exhausted");
  DEST_NSSAI->ran_param_val.strct->sz_ran_param_struct = 2;
  DEST_NSSAI->ran_param_val.strct->ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  // SST, ELEMENT (S-NSSAI -> SST)
  seq_ran_param_t* SST = &DEST_NSSAI->ran_param_val.strct->ran_param_struct[0];
  SST->ran_param_id = DEST_SST_TEMP;
  SST->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  SST->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(SST->ran_param_val.flag_false != NULL && "Memory exhausted");
  SST->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  byte_array_t sst = cp_str_to_ba(sst_str); //TODO
  SST->ran_param_val.flag_false->octet_str_ran.len = sst.len;
  SST->ran_param_val.flag_false->octet_str_ran.buf = sst.buf;

  // SD, ELEMENT (S-NSSAI -> SD)
  seq_ran_param_t* SD = &DEST_NSSAI->ran_param_val.strct->ran_param_struct[1];
  SD->ran_param_id = DEST_SD_TEMP;
  SD->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  SD->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(SD->ran_param_val.flag_false != NULL && "Memory exhausted");
  SD->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  // char sd_str[] = "0";
  byte_array_t sd = cp_str_to_ba(sd_str); //TODO
  SD->ran_param_val.flag_false->octet_str_ran.len = sd.len;
  SD->ran_param_val.flag_false->octet_str_ran.buf = sd.buf;


  return;

}


e2sm_rc_ctrl_msg_frmt_1_t gen_rc_ctrl_msg_frmt_1_HO_slice_level(const char* sst_str, const char* sd_str)
{
  e2sm_rc_ctrl_msg_frmt_1_t dst = {0};


  // Wo dont have any specific standard for HO Slice level. 
  // In this case we use a simple msg


  dst.sz_ran_param = 1;
  dst.ran_param = calloc(1, sizeof(seq_ran_param_t));
  assert(dst.ran_param != NULL && "Memory exhausted");
  gen_HO_slice_level(&dst.ran_param[0], sst_str, sd_str);

  return dst;
}


// Parse a single CSV line to extract UE ID, PRB DL, and Throughput DL
bool parse_csv_line(const char* line, char* ue_id, int* prb_dl, double* thr_dl) {
    if (strstr(line, "amf_ue_ngap_id")) {
        sscanf(line, "%*[^,],%*[^,],%*[^,],%s", ue_id);
        return true;
    } else if (strstr(line, "RRU.PrbTotDl")) {
        sscanf(line, "%*[^,],%d", prb_dl);
        return true;
    } else if (strstr(line, "DRB.UEThpDl")) {
        sscanf(line, "%*[^,],%lf", thr_dl);
        return true;
    }
    return false;
}

// Fetch UE details from the additional CSV file
ue_details_t* get_ue_details(const char* ue_id, const char* file_path) {
    static bool file_not_found_reported = false;
    FILE* file = fopen(file_path, "r");

    if (!file) {
        if (!file_not_found_reported) {
            printf("UE info file %s not found. Ensure the file exists.\n", file_path);
            file_not_found_reported = true;
        }
        return NULL;
    }

    file_not_found_reported = false; // Reset if file is now found

    char line[MAX_LINE_LENGTH];
    ue_details_t* details = malloc(sizeof(ue_details_t));

    // Skip header line
    fgets(line, MAX_LINE_LENGTH, file);

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        char id[32];
        sscanf(line, "%d,%d,%d,%d,%d,%d,%d",
               &details->amf_ue_ngap_id,
               &details->mcc,
               &details->mnc,
               &details->mnc_digit_len,
               &details->amf_region_id,
               &details->amf_set_id,
               &details->amf_ptr);

        sprintf(id, "%d", details->amf_ue_ngap_id);
        if (strcmp(id, ue_id) == 0) {
            fclose(file);
            return details;
        }
    }

    fclose(file);
    free(details);
    return NULL;
}

// Simulate triggering a function with the UE details
void trigger_function(const ue_details_t* ue, e2_node_arr_xapp_t nodes) {
    printf("Triggering function with the following UE structure:\n");
    printf("AMF_UE_NGAP_ID: %d\n", ue->amf_ue_ngap_id);
    printf("MCC: %d\n", ue->mcc);
    printf("MNC: %d\n", ue->mnc);
    printf("MNC_DIGIT_LEN: %d\n", ue->mnc_digit_len);
    printf("AMF_REGION_ID: %d\n", ue->amf_region_id);
    printf("AMF_SET_ID: %d\n", ue->amf_set_id);
    printf("AMF_PTR: %d\n", ue->amf_ptr);

    if (nodes.len <= 0) {
        printf("No E2 nodes available. Cannot perform handover.\n");
        return;
    }

    ue_id_e2sm_t ue_id = {
        .type = GNB_UE_ID_E2SM,
        .gnb = {
            .amf_ue_ngap_id = ue->amf_ue_ngap_id,
            .guami = {
                .plmn_id = {
                    .mcc = ue->mcc,
                    .mnc = ue->mnc,
                    .mnc_digit_len = ue->mnc_digit_len
                },
                .amf_region_id = ue->amf_region_id,
                .amf_set_id = ue->amf_set_id,
                .amf_ptr = ue->amf_ptr
            }
        }
    };
    // Call the external API to perform the handover
    perform_handover(ue_id, nodes);
}

void perform_handover(const ue_id_e2sm_t ue_id, e2_node_arr_xapp_t nodes) {
    printf("Performing handover\n");

    rc_ctrl_req_data_t rc_ctrl = {0};

    rc_ctrl.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id, 3, HO_control);
    rc_ctrl.msg = gen_rc_ctrl_HO_slice_level_msg(FORMAT_1_E2SM_RC_CTRL_MSG, "2", "2");

    printf("Sending HO control message...\n");
    for (size_t i = 0; i < nodes.len; ++i) {
        control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl);
    }
    free_rc_ctrl_req_data(&rc_ctrl);
}