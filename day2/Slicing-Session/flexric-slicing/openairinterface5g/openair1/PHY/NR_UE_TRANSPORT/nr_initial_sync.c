/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file nr_initial_sync.c
 * \brief Routines for initial UE synchronization procedure (PSS,SSS,PBCH and frame format detection)
 * \author R. Knopp, F. Kaltenberger
 * \date 2011
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr,kaltenberger@eurecom.fr
 * \note
 * \warning
 */
#include "PHY/types.h"
#include "PHY/defs_nr_UE.h"
#include "PHY/MODULATION/modulation_UE.h"
#include "nr_transport_proto_ue.h"
#include "PHY/NR_UE_ESTIMATION/nr_estimation.h"
#include "SCHED_NR_UE/defs.h"
#include "common/utils/LOG/vcd_signal_dumper.h"
#include "common/utils/nr/nr_common.h"

#include "common_lib.h"
#include <math.h>

#include "PHY/NR_REFSIG/pss_nr.h"
#include "PHY/NR_REFSIG/sss_nr.h"
#include "PHY/NR_REFSIG/refsig_defs_ue.h"
#include "PHY/TOOLS/tools_defs.h"

//static  nfapi_nr_config_request_t config_t;
//static  nfapi_nr_config_request_t* config =&config_t;
// #define DEBUG_INITIAL_SYNCH
#define DUMP_PBCH_CH_ESTIMATES 0

// structure used for multiple SSB detection
typedef struct NR_UE_SSB {
  uint i_ssb; // i_ssb between 0 and 7 (it corresponds to ssb_index only for Lmax=4,8)
  uint n_hf; // n_hf = 0,1 for Lmax =4 or n_hf = 0 for Lmax =8,64
  double metric; // metric to order SSB hypothesis
} NR_UE_SSB;

static int ssb_sort(const void *a, const void *b)
{
  return ((NR_UE_SSB *)b)->metric - ((NR_UE_SSB *)a)->metric;
}

static bool nr_pbch_detection(const UE_nr_rxtx_proc_t *proc,
                              PHY_VARS_NR_UE *ue,
                              int pbch_initial_symbol,
                              c16_t rxdataF[][ue->frame_parms.samples_per_slot_wCP])
{
  NR_DL_FRAME_PARMS *frame_parms = &ue->frame_parms;

  const int N_L = (frame_parms->Lmax == 4) ? 4 : 8;
  const int N_hf = (frame_parms->Lmax == 4) ? 2 : 1;
  NR_UE_SSB best_ssb[N_L * N_hf];
  NR_UE_SSB *current_ssb = best_ssb;
  // loops over possible pbch dmrs cases to retrieve best estimated i_ssb (and n_hf for Lmax=4) for multiple ssb detection
  start_meas(&ue->dlsch_channel_estimation_stats);
  for (int hf = 0; hf < N_hf; hf++) {
    for (int l = 0; l < N_L; l++) {
      // computing correlation between received DMRS symbols and transmitted sequence for current i_ssb and n_hf
      cd_t cumul = {0};
      for (int i = pbch_initial_symbol; i < pbch_initial_symbol + 3; i++) {
        c32_t meas = nr_pbch_dmrs_correlation(ue, proc, i, i - pbch_initial_symbol, ue->nr_gold_pbch[hf][l], rxdataF);
        csum(cumul, cumul, meas);
      }
      *current_ssb = (NR_UE_SSB){.i_ssb = l, .n_hf = hf, .metric = squaredMod(cumul)};
      current_ssb++;
    }
  }
  qsort(best_ssb, N_L * N_hf, sizeof(NR_UE_SSB), ssb_sort);
  stop_meas(&ue->dlsch_channel_estimation_stats);

  const int nb_ant = frame_parms->nb_antennas_rx;
  for (NR_UE_SSB *ssb = best_ssb; ssb < best_ssb + N_L * N_hf; ssb++) {
    start_meas(&ue->dlsch_channel_estimation_stats);
    // computing channel estimation for selected best ssb
    const int estimateSz = frame_parms->symbols_per_slot * frame_parms->ofdm_symbol_size;
    __attribute__((aligned(32))) c16_t dl_ch_estimates[nb_ant][estimateSz];
    __attribute__((aligned(32))) c16_t dl_ch_estimates_time[nb_ant][frame_parms->ofdm_symbol_size];

    for(int i=pbch_initial_symbol; i<pbch_initial_symbol+3;i++)
      nr_pbch_channel_estimation(ue,
                                 &ue->frame_parms,
                                 estimateSz,
                                 dl_ch_estimates,
                                 dl_ch_estimates_time,
                                 proc,
                                 i,
                                 i - pbch_initial_symbol,
                                 ssb->i_ssb,
                                 ssb->n_hf,
                                 rxdataF,
                                 false,
                                 frame_parms->Nid_cell);

    stop_meas(&ue->dlsch_channel_estimation_stats);
    fapiPbch_t result = {0};
    if (0 == nr_rx_pbch(ue, proc, estimateSz, dl_ch_estimates, frame_parms, ssb->i_ssb, &result, rxdataF)) {
      if (DUMP_PBCH_CH_ESTIMATES) {
        write_output("pbch_ch_estimates.m", "pbch_ch_estimates", dl_ch_estimates, nb_ant * estimateSz, 1, 1);
        write_output("pbch_ch_estimates_time.m",
                     "pbch_ch_estimates_time",
                     dl_ch_estimates_time,
                     nb_ant * frame_parms->ofdm_symbol_size,
                     1,
                     1);
      }
      LOG_I(PHY, "[UE%d] Initial sync: pbch decoded sucessfully, ssb index %d\n", ue->Mod_id, frame_parms->ssb_index);
      return true;
    }
  }

  LOG_W(PHY, "[UE%d] Initial sync: pbch not decoded, ssb index %d\n", ue->Mod_id, frame_parms->ssb_index);
  return false;
}

nr_initial_sync_t nr_initial_sync(UE_nr_rxtx_proc_t *proc, PHY_VARS_NR_UE *ue, int n_frames, int sa)
{
  int32_t sync_pos, sync_pos_frame; // k_ssb, N_ssb_crb, sync_pos2,

  NR_DL_FRAME_PARMS *fp = &ue->frame_parms;
  nr_initial_sync_t ret = {.cell_detected = false};

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_NR_INITIAL_UE_SYNC, VCD_FUNCTION_IN);

  LOG_D(PHY, "nr_initial sync ue RB_DL %d\n", fp->N_RB_DL);

  /*   Initial synchronisation
   *
   *                                 1 radio frame = 10 ms
   *     <--------------------------------------------------------------------------->
   *     -----------------------------------------------------------------------------
   *     |                                 Received UE data buffer                    |
   *     ----------------------------------------------------------------------------
   *                     --------------------------
   *     <-------------->| pss | pbch | sss | pbch |
   *                     --------------------------
   *          sync_pos            SS/PBCH block
   */

  const uint32_t rxdataF_sz = ue->frame_parms.samples_per_slot_wCP;
  __attribute__((aligned(32))) c16_t rxdataF[ue->frame_parms.nb_antennas_rx][rxdataF_sz];

  // initial sync performed on two successive frames, if pbch passes on first frame, no need to process second frame
  // only one frame is used for symulation tools
  for (int frame_id = 0; frame_id < n_frames && !ret.cell_detected; frame_id++) {
    /* process pss search on received buffer */
    sync_pos = pss_synchro_nr(ue, frame_id, NO_RATE_CHANGE);
    if (sync_pos < fp->nb_prefix_samples)
      continue;

    ue->ssb_offset = sync_pos - fp->nb_prefix_samples;

#ifdef DEBUG_INITIAL_SYNCH
    LOG_I(PHY, "[UE%d] Initial sync : Estimated PSS position %d, Nid2 %d\n", ue->Mod_id, sync_pos, ue->common_vars.nid2);
    LOG_I(PHY, "sync_pos %d ssb_offset %d \n", sync_pos, ue->ssb_offset);
#endif
    /* check that SSS/PBCH block is continuous inside the received buffer */
    if (ue->ssb_offset + NR_N_SYMBOLS_SSB * (fp->ofdm_symbol_size + fp->nb_prefix_samples) >= fp->samples_per_frame) {
      LOG_I(PHY, "Can't try to decode SSS from PSS position, will retry (PSS circular buffer wrapping): sync_pos %d\n", sync_pos);
      continue;
    }

    // digital compensation of FFO for SSB symbols
    if (ue->UE_fo_compensation) {
      double s_time = 1 / (1.0e3 * fp->samples_per_subframe); // sampling time
      double off_angle = -2 * M_PI * s_time * (ue->common_vars.freq_offset); // offset rotation angle compensation per sample

      // In SA we need to perform frequency offset correction until the end of buffer because we need to decode SIB1
      // and we do not know yet in which slot it goes.

      for (int n = frame_id * fp->samples_per_frame; n < (frame_id + 1) * fp->samples_per_frame; n++) {
        for (int ar = 0; ar < fp->nb_antennas_rx; ar++) {
          const double re = ue->common_vars.rxdata[ar][n].r;
          const double im = ue->common_vars.rxdata[ar][n].i;
          ue->common_vars.rxdata[ar][n].r = (short)(round(re * cos(n * off_angle) - im * sin(n * off_angle)));
          ue->common_vars.rxdata[ar][n].i = (short)(round(re * sin(n * off_angle) + im * cos(n * off_angle)));
        }
      }
    }

    /* slot_fep function works for lte and takes into account begining of frame with prefix for subframe 0 */
    /* for NR this is not the case but slot_fep is still used for computing FFT of samples */
    /* in order to achieve correct processing for NR prefix samples is forced to 0 and then restored after function call */
    /* symbol number are from beginning of SS/PBCH blocks as below:  */
    /*    Signal            PSS  PBCH  SSS  PBCH                     */
    /*    symbol number      0     1    2    3                       */
    /* time samples in buffer rxdata are used as input of FFT -> FFT results are stored in the frequency buffer rxdataF */
    /* rxdataF stores SS/PBCH from beginning of buffers in the same symbol order as in time domain */

    for (int i = 0; i < NR_N_SYMBOLS_SSB; i++)
      nr_slot_fep_init_sync(ue, proc, i, frame_id * fp->samples_per_frame + ue->ssb_offset, false, rxdataF, link_type_dl);

#ifdef DEBUG_INITIAL_SYNCH
    LOG_I(PHY, "Calling sss detection (normal CP)\n");
#endif

    int freq_offset_sss = 0;
    int32_t metric_tdd_ncp = 0;
    uint8_t phase_tdd_ncp;
    ret.cell_detected = rx_sss_nr(ue, proc, &metric_tdd_ncp, &phase_tdd_ncp, &freq_offset_sss, rxdataF);
    
    // digital compensation of FFO for SSB symbols
    if (freq_offset_sss && ue->UE_fo_compensation) {
      double s_time = 1 / (1.0e3 * fp->samples_per_subframe); // sampling time
      double off_angle = -2 * M_PI * s_time * freq_offset_sss; // offset rotation angle compensation per sample

      // In SA we need to perform frequency offset correction until the end of buffer because we need to decode SIB1
      // and we do not know yet in which slot it goes.
      for (int n = frame_id * fp->samples_per_frame; n < (frame_id + 1) * fp->samples_per_frame; n++) {
        for (int ar = 0; ar < fp->nb_antennas_rx; ar++) {
          const double re = ue->common_vars.rxdata[ar][n].r;
          const double im = ue->common_vars.rxdata[ar][n].i;
          ue->common_vars.rxdata[ar][n].r = (short)(round(re * cos(n * off_angle) - im * sin(n * off_angle)));
          ue->common_vars.rxdata[ar][n].i = (short)(round(re * sin(n * off_angle) + im * cos(n * off_angle)));
        }
      }
      ue->common_vars.freq_offset += freq_offset_sss;
    }

    if (ret.cell_detected) { // we got sss channel
      nr_gold_pbch(ue);
      ret.cell_detected = nr_pbch_detection(proc, ue, 1, rxdataF); // start pbch detection at first symbol after pss
    }

    if (ret.cell_detected) {
      // sync at symbol ue->symbol_offset
      // computing the offset wrt the beginning of the frame
      int mu = fp->numerology_index;
      // number of symbols with different prefix length
      // every 7*(1<<mu) symbols there is a different prefix length (38.211 5.3.1)
      int n_symb_prefix0 = (ue->symbol_offset / (7 * (1 << mu))) + 1;
      sync_pos_frame = n_symb_prefix0 * (fp->ofdm_symbol_size + fp->nb_prefix_samples0)
                       + (ue->symbol_offset - n_symb_prefix0) * (fp->ofdm_symbol_size + fp->nb_prefix_samples);
      // for a correct computation of frame number to sync with the one decoded at MIB we need to take into account in which of
      // the n_frames we got sync
      ue->init_sync_frame = n_frames - 1 - frame_id;

      // compute the scramblingID_pdcch and the gold pdcch
      ue->scramblingID_pdcch = fp->Nid_cell;
      nr_gold_pdcch(ue, fp->Nid_cell);

      // compute the scrambling IDs for PDSCH DMRS
      for (int i = 0; i < NR_NB_NSCID; i++) {
        ue->scramblingID_dlsch[i] = fp->Nid_cell;
        nr_gold_pdsch(ue, i, ue->scramblingID_dlsch[i]);
      }

      nr_init_csi_rs(fp, ue->nr_csi_info->nr_gold_csi_rs, fp->Nid_cell);

      // initialize the pusch dmrs
      for (int i = 0; i < NR_NB_NSCID; i++) {
        ue->scramblingID_ulsch[i] = fp->Nid_cell;
        nr_init_pusch_dmrs(ue, ue->scramblingID_ulsch[i], i);
      }

      // we also need to take into account the shift by samples_per_frame in case the if is true
      if (ue->ssb_offset < sync_pos_frame) {
        ret.rx_offset = fp->samples_per_frame - sync_pos_frame + ue->ssb_offset;
        ue->init_sync_frame += 1;
      } else
        ret.rx_offset = ue->ssb_offset - sync_pos_frame;
    }

#ifdef DEBUG_INITIAL_SYNCH
    LOG_I(PHY,
          "TDD Normal prefix: CellId %d metric %d, phase %d, pbch detected %d, measured offset %d\n",
          fp->Nid_cell,
          metric_tdd_ncp,
          phase_tdd_ncp,
          ret.cell_detected,
          ret.rx_offset);
#endif
  }

  /* Consider this is a false detection if the offset is > 1000 Hz
     Not to be used now that offest estimation is in place
     if( (abs(ue->common_vars.freq_offset) > 150) && (ret == 0) )
     {
     ret=-1;
     LOG_E(HW, "Ignore MIB with high freq offset [%d Hz] estimation \n",ue->common_vars.freq_offset);
     }*/

  if (ret.cell_detected) { // PBCH found so indicate sync to higher layers and configure frame parameters

    //#ifdef DEBUG_INITIAL_SYNCH

    LOG_I(PHY, "[UE%d] In synch, rx_offset %d samples\n", ue->Mod_id, ret.rx_offset);

    //#endif

    if (ue->UE_scan_carrier == 0) {
#if UE_AUTOTEST_TRACE
      LOG_I(PHY,
            "[UE  %d] AUTOTEST Cell Sync : rx_offset %d, freq_offset %d \n",
            ue->Mod_id,
            ue->rx_offset,
            ue->common_vars.freq_offset);
#endif

      // send sync status to higher layers later when timing offset converge to target timing
    }

    LOG_I(PHY, "[UE %d] Measured Carrier Frequency offset %d Hz\n", ue->Mod_id, ue->common_vars.freq_offset);
  } else {
#ifdef DEBUG_INITIAL_SYNC
    LOG_I(PHY,"[UE%d] Initial sync : PBCH not ok\n",ue->Mod_id);
    LOG_I(PHY, "[UE%d] Initial sync : Estimated PSS position %d, Nid2 %d\n", ue->Mod_id, sync_pos, ue->common_vars.nid2);
    LOG_I(PHY,"[UE%d] Initial sync : Estimated Nid_cell %d, Frame_type %d\n",ue->Mod_id,
          frame_parms->Nid_cell,frame_parms->frame_type);
#endif
  }

  // gain control
  if (!ret.cell_detected) { // we are not synched, so we cannot use rssi measurement (which is based on channel estimates)
    int rx_power = 0;

    // do a measurement on the best guess of the PSS
    // for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++)
    //  rx_power += signal_energy(&ue->common_vars.rxdata[aarx][sync_pos2],
    //			frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples);

    /*
    // do a measurement on the full frame
    for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++)
    rx_power += signal_energy(&ue->common_vars.rxdata[aarx][0],
    frame_parms->samples_per_subframe*10);
    */

    // we might add a low-pass filter here later
    ue->measurements.rx_power_avg[0] = rx_power/fp->nb_antennas_rx;

    ue->measurements.rx_power_avg_dB[0] = dB_fixed(ue->measurements.rx_power_avg[0]);

#ifdef DEBUG_INITIAL_SYNCH
    LOG_I(PHY, "[UE%d] Initial sync : Estimated power: %d dB\n", ue->Mod_id, ue->measurements.rx_power_avg_dB[0]);
#endif
  } else {
    LOG_A(PHY, "Initial sync successful\n");
  }
  //  exit_fun("debug exit");
  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_NR_INITIAL_UE_SYNC, VCD_FUNCTION_OUT);
  return ret;
}
