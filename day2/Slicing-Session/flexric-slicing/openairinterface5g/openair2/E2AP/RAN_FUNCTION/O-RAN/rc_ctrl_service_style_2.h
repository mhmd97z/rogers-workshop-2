#ifndef OPENAIRINTERFACE_RC_CTRL_SERVICE_STYLE_2_H
#define OPENAIRINTERFACE_RC_CTRL_SERVICE_STYLE_2_H

#include "openair2/E2AP/flexric/src/sm/rc_sm/ie/ir/lst_ran_param.h"
#include "openair2/E2AP/flexric/src/sm/rc_sm/ie/ir/ran_param_list.h"
#include "openair2/E2AP/flexric/src/sm/rc_sm/ie/ir/ran_parameter_value.h"
#include "openair2/E2AP/flexric/src/sm/rc_sm/ie/ir/ran_parameter_value_type.h"
#include "openair2/E2AP/flexric/src/sm/slice_sm/ie/slice_data_ie.h"
#include "openair2/E2AP/flexric/src/lib/sm/ie/ue_id.h"

bool add_mod_rc_slice(int mod_id, size_t slices_len, ran_param_list_t* lst);
bool rc_HO_slice_level(ue_id_e2sm_t* RC_ue_id, nssai_t RC_nssai, int mod_id);


typedef enum{
  DRX_parameter_configuration_7_6_3_1 = 1,
  SR_periodicity_configuration_7_6_3_1 = 2,
  SPS_parameters_configuration_7_6_3_1 = 3,
  Configured_grant_control_7_6_3_1 = 4,
  CQI_table_configuration_7_6_3_1 = 5,
  Slice_level_PRB_quotal_7_6_3_1 = 6,
} rc_ctrl_service_style_2_act_id_e;

typedef enum {
  RRM_Policy_Ratio_List_8_4_3_6 = 1,
  RRM_Policy_Ratio_Group_8_4_3_6 = 2,
  RRM_Policy_8_4_3_6 = 3,
  RRM_Policy_Member_List_8_4_3_6 = 4,
  RRM_Policy_Member_8_4_3_6 = 5,
  PLMN_Identity_8_4_3_6 = 6,
  S_NSSAI_8_4_3_6 = 7,
  SST_8_4_3_6 = 8,
  SD_8_4_3_6 = 9,
  Min_PRB_Policy_Ratio_8_4_3_6 = 10,
  Max_PRB_Policy_Ratio_8_4_3_6 = 11,
  Dedicated_PRB_Policy_Ratio_8_4_3_6 = 12,
} slice_level_PRB_quota_param_id_e;

typedef enum {
    HO_INFO = 1,
    DEST_NSSAI_TEMP = 2,
    DEST_SST_TEMP= 3,
    DEST_SD_TEMP = 4,
    PLMN_Identity_TEMP = 5,
} HO_slice_level_param_id_e;
#endif // OPENAIRINTERFACE_RC_CTRL_SERVICE_STYLE_2_H
