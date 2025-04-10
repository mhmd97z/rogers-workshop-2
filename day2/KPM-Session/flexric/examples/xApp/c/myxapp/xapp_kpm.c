

#include "kpm.h"
#include <inttypes.h>


// extern ue_id_e2sm_t ue;

 // 1: Connect to the FlexRIC
 // 3: Send KPM subscription

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
  // START KPM SLICE 1
  ////////////

  sm_ans_xapp_t* hndl = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  assert(hndl != NULL);

  int const KPM_ran_function = 2;
  e2_node_connected_xapp_t* n = &nodes.n[0]; // There is just one RAN
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

  ////////////
  // RELEASE KPM OF SLICE 1
  ////////////
  rm_report_sm_xapp_api(hndl[0].u.handle);

  ////////////
  // Stop the xApp
  ////////////
  getchar();

  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");

}

