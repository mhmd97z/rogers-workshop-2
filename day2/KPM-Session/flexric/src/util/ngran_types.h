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
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file common/ngran_types.h
* \brief Definitions for NGRAN node types
* \author R. Knopp
* \date 2018
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/

#ifndef __NGRAN_TYPES_H__
#define __NGRAN_TYPES_H__

typedef enum {
  ngran_eNB       = 0,
  ngran_ng_eNB    = 1,
  ngran_gNB       = 2,
  ngran_eNB_CU    = 3,
  ngran_ng_eNB_CU = 4,
  ngran_gNB_CU    = 5,
  ngran_eNB_DU    = 6,
  ngran_gNB_DU    = 7,
  ngran_eNB_MBMS_STA  = 8,
  ngran_gNB_CUCP  = 9,
  ngran_gNB_CUUP  = 10,

  END_NGRAN_NODE_TYPE
} ngran_node_t;

char* get_ngran_name(ngran_node_t n);


#define NODE_IS_MONOLITHIC(nOdE_TyPe) ((nOdE_TyPe) == ngran_eNB    || (nOdE_TyPe) == ngran_ng_eNB    || (nOdE_TyPe) == ngran_gNB)
#define NODE_IS_CU(nOdE_TyPe)         ((nOdE_TyPe) == ngran_eNB_CU || (nOdE_TyPe) == ngran_ng_eNB_CU || (nOdE_TyPe) == ngran_gNB_CU || (nOdE_TyPe) == ngran_gNB_CUCP || (nOdE_TyPe) == ngran_gNB_CUUP)
#define NODE_IS_DU(nOdE_TyPe)         ((nOdE_TyPe) == ngran_eNB_DU || (nOdE_TyPe) == ngran_gNB_DU)
#define NODE_IS_MBMS(nOdE_TyPe)       ((nOdE_TyPe) == ngran_eNB_MBMS_STA)
#define NODE_IS_CUUP(nOdE_TyPe) ((nOdE_TyPe) == ngran_gNB_CUUP)
#define GTPV1_U_PORT_NUMBER (2152)

typedef enum { non_dynamic, dynamic } fiveQI_type_t;
typedef enum { CPtype = 0, UPtype } E1_t;

#define maxSRBs 4
#endif
