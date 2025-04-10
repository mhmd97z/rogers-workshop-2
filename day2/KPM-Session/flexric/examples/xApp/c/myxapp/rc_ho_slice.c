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