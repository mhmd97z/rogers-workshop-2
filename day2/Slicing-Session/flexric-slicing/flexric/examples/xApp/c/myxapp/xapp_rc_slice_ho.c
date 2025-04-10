

#include "kpm.h"
#include "rc_slice.h"
#include "rc_ho_slice.h"
#include <assert.h>
#include <inttypes.h>
#include <cjson/cJSON.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>


extern ue_id_e2sm_t ue;

// Global variables
const char* monitored_csv = "/home/user/workshop_slicing/flexric-slicing/flexric/examples/xApp/c/myxapp/measurements.csv";
const char* ue_info_csv = "/home/user/workshop_slicing/flexric-slicing/flexric/examples/xApp/c/myxapp/ue_data.csv";
const char* slice_config = "/home/user/workshop_slicing/flexric-slicing/flexric/examples/xApp/c/myxapp/config.json";


void print_menu() {
    printf("\nPlease choose an option:\n");
    printf("1. Create Slices\n");
    printf("2. Send Handover Request\n");
    printf("Enter your choice: ");
}


char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    assert(file != NULL && "Could not open file");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';
    return data;
}

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
  // There is just one RAN, TODO: extend to support multiple RANs
  assert(nodes.len == 1 && "There should be one RAN connected to the FlexRIC"); 
  printf("Connected E2 nodes = %d\n", nodes.len);
  print_menu();
  int choice = 0;
  scanf("%d", &choice);
  ue_id_e2sm_t ue_id = gen_rc_ue_id(GNB_UE_ID_E2SM); // Generate random UE ID
  sm_ans_xapp_t* hndl = calloc(2, sizeof(sm_ans_xapp_t));
  assert(hndl != NULL && "Memory allocation failed");
  e2_node_connected_xapp_t* n = &nodes.n[0];
  size_t idx = 0;
  int const KPM_ran_function = 2;

  if (choice == 1) {
    ////////////
    // Read configuration file
    ////////////
    char* data = read_file(slice_config); 
    assert(data != NULL && "Could not read configuration file");

    cJSON* json = cJSON_Parse(data);
    assert(json != NULL && "Could not parse JSON configuration file");

    int num_slice = cJSON_GetObjectItem(json, "num_slice")->valueint;
    assert(num_slice > 0 && "Number of slices must be greater than 0");
    printf("Number of slices: %d\n", num_slice);
    
    cJSON* sst_array = cJSON_GetObjectItem(json, "sst_str");
    cJSON* sd_array = cJSON_GetObjectItem(json, "sd_str");
    cJSON* dedicated_ratio_prb_array = cJSON_GetObjectItem(json, "dedicated_ratio_prb");
    const char** sst_str = calloc(num_slice, sizeof(char*));
    const char** sd_str = calloc(num_slice, sizeof(char*));
    int* dedicated_ratio_prb = calloc(num_slice, sizeof(int));

    for (int i = 0; i < num_slice; i++) {
        sst_str[i] = cJSON_GetArrayItem(sst_array, i)->valuestring;
        sd_str[i] = cJSON_GetArrayItem(sd_array, i)->valuestring;
        dedicated_ratio_prb[i] = cJSON_GetArrayItem(dedicated_ratio_prb_array, i)->valueint;
        // Print extracted values for debugging
        printf("Slice %d:\n", i+1);
        printf("  SST: %s\n", sst_str[i]);
        printf("  SD: %s\n", sd_str[i]);
        printf("  Dedicated Ratio PRB: %d\n", dedicated_ratio_prb[i]);
    }
    ////////////
    // Send RC Slice to create slices
    ////////////
    rc_ctrl_req_data_t rc_ctrl = {0};
    printf("here is the ue_id generated in creating slices %llu\n", (unsigned long long)ue_id.gnb.amf_ue_ngap_id);

    rc_ctrl.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id, 2, Slice_level_PRB_quotal_7_6_3_1);
    rc_ctrl.msg = gen_rc_ctrl_slice_level_PRB_quata_msg(FORMAT_1_E2SM_RC_CTRL_MSG, sst_str, sd_str, dedicated_ratio_prb, num_slice);
    for(size_t i =0; i < nodes.len; ++i){
      control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl);
    }
    free_rc_ctrl_req_data(&rc_ctrl);

    free(sst_str);
    free(sd_str);
    free(dedicated_ratio_prb);
    cJSON_Delete(json);
    free(data);

    printf("The Slices are created. \n");
    // getchar();


    ////////////
    // START KPM SLICE 1
    ////////////

    idx = find_sm_idx(n->rf, n->len_rf, eq_sm, KPM_ran_function);
    assert(n->rf[idx].defn.type == KPM_RAN_FUNC_DEF_E && "KPM is not the received RAN Function");
    // if REPORT Service is supported by E2 node, send SUBSCRIPTION
    if (n->rf[idx].defn.kpm.ric_report_style_list != NULL) {

      FILE *file = fopen(monitored_csv, "w");  // Open the file in write mode
      fprintf(file, "KPM ind_msg latency,UE ID type,gNB,amf_ue_ngap_id,ran_ue_id,DRB.PdcpSduVolumeDL,DRB.PdcpSduVolumeUL,DRB.RlcSduDelayDl,DRB.UEThpDl,DRB.UEThpUl,RRU.PrbTotDl,RRU.PrbTotUl\n");
      fclose(file);

      FILE *csv_file = fopen(ue_info_csv, "w");
      // Write CSV header if the file is empty
      if (ftell(csv_file) == 0) {
          fprintf(csv_file, "AMF_UE_NGAP_ID,MCC,MNC,MNC_DIGIT_LEN,AMF_REGION_ID,AMF_SET_ID,AMF_PTR\n");
      }
      fclose(csv_file);

      // Generate KPM SUBSCRIPTION message
      int value = 1;
      kpm_sub_data_t kpm_sub = gen_kpm_subs(&n->rf[idx].defn.kpm,value);

      hndl[0] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
      assert(hndl[0].success == true && "KPM Subscription failed");

      free_kpm_sub_data(&kpm_sub);

      sleep(60);

      printf("Realeasing the subscribtion to the slice 1 for KPM. \n");
      rm_report_sm_xapp_api(hndl[0].u.handle);
      
    }


    // getchar();

    } else if (choice == 2) {

    ////////////
    // Monitoring and Send RC Slice HO
    ////////////

    printf("The Slices are created. lets test the inter slice handover\n");

    idx = find_sm_idx(n->rf, n->len_rf, eq_sm, KPM_ran_function);
    assert(n->rf[idx].defn.type == KPM_RAN_FUNC_DEF_E && "KPM is not the received RAN Function");
    // if REPORT Service is supported by E2 node, send SUBSCRIPTION
    if (n->rf[idx].defn.kpm.ric_report_style_list != NULL) {

      FILE *file = fopen(monitored_csv, "w");  // Open the file in write mode
      fprintf(file, "KPM ind_msg latency,UE ID type,gNB,amf_ue_ngap_id,ran_ue_id,DRB.PdcpSduVolumeDL,DRB.PdcpSduVolumeUL,DRB.RlcSduDelayDl,DRB.UEThpDl,DRB.UEThpUl,RRU.PrbTotDl,RRU.PrbTotUl\n");
      fclose(file);

      FILE *csv_file = fopen(ue_info_csv, "w");
      // Write CSV header if the file is empty
      if (ftell(csv_file) == 0) {
          fprintf(csv_file, "AMF_UE_NGAP_ID,MCC,MNC,MNC_DIGIT_LEN,AMF_REGION_ID,AMF_SET_ID,AMF_PTR\n");
      }
      fclose(csv_file);

      // Generate KPM SUBSCRIPTION message
      int value = 1;
      kpm_sub_data_t kpm_sub = gen_kpm_subs(&n->rf[idx].defn.kpm,value);

      hndl[0] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
      assert(hndl[0].success == true && "KPM Subscription failed");

      free_kpm_sub_data(&kpm_sub);


      sleep(5);    
      const char* sst_str_HO = "2";
      const char* sd_str_HO = "2";
      // const int amf_ue_ngap_id_HO = 6;

      rc_ctrl_req_data_t rc_ctrl_ = {0};
      ue_id = ue; // find the specific UE!!!!!
      printf("here is the amf_ue_ngap_id generated in handover: %llu\n", (unsigned long long)ue_id.gnb.amf_ue_ngap_id);
      rc_ctrl_.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id, 3, HO_control);
      rc_ctrl_.msg = gen_rc_ctrl_HO_slice_level_msg(FORMAT_1_E2SM_RC_CTRL_MSG, sst_str_HO, sd_str_HO);
      
      printf("HO finished\n");

      int64_t st = time_now_us();
      for(size_t i =0; i < nodes.len; ++i){
        control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl_);
      }
      printf("[xApp]: Control Loop Latency: %ld us\n", time_now_us() - st);
      free_rc_ctrl_req_data(&rc_ctrl_);

      // getchar();

      ////////////
      // RELEASE KPM OF SLICE 1
      ////////////
      printf("Realeasing the subscribtion to the slice 1 for KPM. \n");
      rm_report_sm_xapp_api(hndl[0].u.handle);
      sleep(8);
      // free(hndl);

      ////////////
      // START KPM SLICE 2
      ////////////
      printf("Subscribing on slice 2 for KPM. \n");

      value = 2;
      kpm_sub = gen_kpm_subs(&n->rf[idx].defn.kpm,value);

      hndl[1] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
      assert(hndl[1].success == true);

      free_kpm_sub_data(&kpm_sub);
      // }
      // printf("Next inter will stop the xApp \n");
      sleep(60);
      
      printf("Realeasing the subscribtion to the slice 1 for KPM. \n");
      rm_report_sm_xapp_api(hndl[1].u.handle);

    ////////////
    // Stop the xApp
    ////////////

    // getchar();
    }
  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");


}

}