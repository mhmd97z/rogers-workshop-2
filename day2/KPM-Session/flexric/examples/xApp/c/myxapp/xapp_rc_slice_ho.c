

#include "kpm.h"
#include "rc_slice.h"
#include "rc_ho_slice.h"
#include <inttypes.h>


extern ue_id_e2sm_t ue;

 // 1: Connect to the FlexRIC
 // 2: Create Slices
 // 3: Send KPM subscription
 // 4: Retrieve UE_ID
 // 5: Send handover request

int main(int argc, char *argv[])
{
  fr_args_t args = init_fr_args(argc, argv);
  //defer({ free_fr_args(&args); });

  

  ////////////
  // Connect to the FlexRIC
  ////////////
  
  //Init the xApp
  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });
  assert(nodes.len > 0);
  printf("Connected E2 nodes = %d\n", nodes.len);

  ////////////
  // Send RC Slice
  ////////////
  const char* sst_str[] = {"1", "2"};
  const char* sd_str[] = {"1", "2"};
  int dedicated_ratio_prb[] = {70, 30};
  int num_slice = 2;

  rc_ctrl_req_data_t rc_ctrl = {0};
  ue_id_e2sm_t ue_id = gen_rc_ue_id(GNB_UE_ID_E2SM);

  rc_ctrl.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id, 2, Slice_level_PRB_quotal_7_6_3_1);
  rc_ctrl.msg = gen_rc_ctrl_slice_level_PRB_quata_msg(FORMAT_1_E2SM_RC_CTRL_MSG, sst_str, sd_str, dedicated_ratio_prb, num_slice);
  for(size_t i =0; i < nodes.len; ++i){
    control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl);
  }
  free_rc_ctrl_req_data(&rc_ctrl);


  printf("The Slices are created. \n");
  getchar();


  ////////////
  // START KPM SLICE 1
  ////////////
  
  sm_ans_xapp_t* hndl = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  assert(hndl != NULL);

  int const KPM_ran_function = 2;
  e2_node_connected_xapp_t* n = &nodes.n[0]; // There is just one RAN, TODO: extend to support multiple RAN
  size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, KPM_ran_function);
  assert(n->rf[idx].defn.type == KPM_RAN_FUNC_DEF_E && "KPM is not the received RAN Function");
  // if REPORT Service is supported by E2 node, send SUBSCRIPTION
  if (n->rf[idx].defn.kpm.ric_report_style_list != NULL) {
    // Generate KPM SUBSCRIPTION message
    int value = 1;
    kpm_sub_data_t kpm_sub = gen_kpm_subs(&n->rf[idx].defn.kpm,value);

    hndl[0] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
    assert(hndl[0].success == true);

    free_kpm_sub_data(&kpm_sub);
  }
  // printf("The NGAP is: %" PRIu64 "\n", ue.gnb.amf_ue_ngap_id);
  // printf("The UE_ID is: %lx\n", *ue.gnb.ran_ue_id);

  getchar();

  ////////////
  // Send RC Slice HO
  ////////////

  printf("The Slices are created. lets test the inter slice handover\n");
  
  const char* sst_str_HO = "2";
  const char* sd_str_HO = "2";

  rc_ctrl_req_data_t rc_ctrl_ = {0};
  // ue_id_e2sm_t ue_id = gen_rc_ue_id(GNB_UE_ID_E2SM);  // find the specific UE!!!!!

  rc_ctrl_.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id, 3, HO_control);
  rc_ctrl_.msg = gen_rc_ctrl_HO_slice_level_msg(FORMAT_1_E2SM_RC_CTRL_MSG, sst_str_HO, sd_str_HO);
  
  printf("HO finished\n");

  int64_t st = time_now_us();
  for(size_t i =0; i < nodes.len; ++i){
    control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl_);
  }
  printf("[xApp]: Control Loop Latency: %ld us\n", time_now_us() - st);
  free_rc_ctrl_req_data(&rc_ctrl_);

  getchar();

  ////////////
  // RELEASE KPM OF SLICE 1
  ////////////

  rm_report_sm_xapp_api(hndl[0].u.handle);
  // free(hndl);

  ////////////
  // START KPM SLICE 2
  ////////////

  // sm_ans_xapp_t* hndl = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  // assert(hndl != NULL);

  // int const KPM_ran_function = 2;
  // e2_node_connected_xapp_t* n = &nodes.n[0]; // There is just one RAN, TODO: extend to support multiple RAN
  // size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, KPM_ran_function);
  // assert(n->rf[idx].defn.type == KPM_RAN_FUNC_DEF_E && "KPM is not the received RAN Function");
  // // if REPORT Service is supported by E2 node, send SUBSCRIPTION
  // if (n->rf[idx].defn.kpm.ric_report_style_list != NULL) {
    // Generate KPM SUBSCRIPTION message
  int value = 2;
  kpm_sub_data_t kpm_sub = gen_kpm_subs(&n->rf[idx].defn.kpm,value);

  hndl[1] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
  assert(hndl[1].success == true);

  free_kpm_sub_data(&kpm_sub);
  // }


  ////////////
  // Stop the xApp
  ////////////

  getchar();
  
  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");


//   ////////////
//   // START RC
//   ////////////

//   // RC Control
//   // CONTROL Service Style 2: Radio Resource Allocation Control
//   // Action ID 6: Slice-level PRB quota
//   // E2SM-RC Control Header Format 1
//   // E2SM-RC Control Message Format 1
//   rc_ctrl_req_data_t rc_ctrl = {0};
//   ue_id_e2sm_t ue_id = gen_rc_ue_id(GNB_UE_ID_E2SM);

//  if (operation == 0) {
//         // Slice creation logic
//   rc_ctrl.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id, 2, Slice_level_PRB_quotal_7_6_3_1);
//   rc_ctrl.msg = gen_rc_ctrl_slice_level_PRB_quata_msg(FORMAT_1_E2SM_RC_CTRL_MSG);

//   int64_t st = time_now_us();
//   for(size_t i =0; i < nodes.len; ++i){
//     control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl);
//   }
//   printf("[xApp]: Control Loop Latency: %ld us\n", time_now_us() - st);
//   free_rc_ctrl_req_data(&rc_ctrl);

//   } else if (operation == 1) {
//       // Handover logic
//   printf("The Slices are created. lets test the inter slice handover\n");
  
//   const char* sst_str = "2";
//   const char* sd_str = "2";

//   rc_ctrl_req_data_t rc_ctrl_ = {0};
//   // ue_id_e2sm_t ue_id = gen_rc_ue_id(GNB_UE_ID_E2SM);  // find the specific UE!!!!!

//   rc_ctrl_.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id, 3, HO_control);
//   rc_ctrl_.msg = gen_rc_ctrl_HO_slice_level_msg(FORMAT_1_E2SM_RC_CTRL_MSG, sst_str, sd_str);
  

//   // We have to find the source node and send Ho request to it. 
//   // Wo wont do it in this case, We send the request to all cells.
//   int64_t st = time_now_us();
//   for(size_t i =0; i < nodes.len; ++i){
//     control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl_);
//   }
//   printf("[xApp]: Control Loop Latency: %ld us\n", time_now_us() - st);
//   free_rc_ctrl_req_data(&rc_ctrl_);

//   // printf("Handover operation completed successfully.\n");
//   }
//   ////////////
//   // END RC
//   ////////////

//   sleep(5);


//   //Stop the xApp
//   while(try_stop_xapp_api() == false)
//     usleep(1000);

//   printf("Test xApp run SUCCESSFULLY\n");

}

