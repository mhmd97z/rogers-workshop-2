#ifndef RC_HO_SLICE_H
#define RC_HO_SLICE_H


#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_struct.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_list.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../../../../src/sm/rc_sm/rc_sm_id.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


// The UE_Id will be in Hdr, not msg.
typedef enum {
    HO_INFO = 1,
    DEST_NSSAI_TEMP = 2,
    DEST_SST_TEMP= 3,
    DEST_SD_TEMP = 4,
    PLMN_Identity_TEMP = 5,
} HO_slice_level_param_id_e;


e2sm_rc_ctrl_msg_t gen_rc_ctrl_HO_slice_level_msg(e2sm_rc_ctrl_msg_e msg_frmt, const char* sst_str, const char* sd_str);
e2sm_rc_ctrl_msg_frmt_1_t gen_rc_ctrl_msg_frmt_1_HO_slice_level(const char* sst_str, const char* sd_str);
void gen_HO_slice_level(seq_ran_param_t* HO_slice_level_msg, const char* sst_str, const char* sd_str);

#endif 