//---------------------------------------------------------------------------//
//                                                                           //
//  This benchmark is an OpenCL C version of the NPB LU code. This OpenCL C  //
//  version is a part of SNU-NPB 2019 developed by the Center for Manycore   //
//  Programming at Seoul National University and derived from the serial     //
//  Fortran versions in "NPB3.3.1-SER" developed by NAS.                     //
//                                                                           //
//  Permission to use, copy, distribute and modify this software for any     //
//  purpose with or without fee is hereby granted. This software is          //
//  provided "as is" without express or implied warranty.                    //
//                                                                           //
//  Information on original NPB 3.3.1, including the technical report, the   //
//  original specifications, source code, results and information on how     //
//  to submit new results, is available at:                                  //
//                                                                           //
//           http://www.nas.nasa.gov/Software/NPB/                           //
//                                                                           //
//  Information on SNU-NPB 2019, including the conference paper and source   //
//  code, is available at:                                                   //
//                                                                           //
//           http://aces.snu.ac.kr                                           //
//                                                                           //
//  Send comments or suggestions for this OpenCL C version to                //
//  snunpb@aces.snu.ac.kr                                                    //
//                                                                           //
//          Center for Manycore Programming                                  //
//          School of Computer Science and Engineering                       //
//          Seoul National University                                        //
//          Seoul 08826, Korea                                               //
//                                                                           //
//          E-mail: snunpb@aces.snu.ac.kr                                    //
//                                                                           //
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Authors: Youngdong Do, Hyung Mo Kim, Pyeongseok Oh, Daeyoung Park,        //
//          and Jaejin Lee                                                   //
//---------------------------------------------------------------------------//

#include <stdio.h>
#include "applu.incl"
#include "timers.h"
#include <math.h>

cl_kernel k_jbu_datagen_sync,
          k_jacu_sync,
          k_buts_BR_sync,
          k_buts_KL_sync;

cl_event  *ev_k_jbu_datagen_sync,
          *ev_k_jacu_sync,
          (*ev_k_buts_BR_sync)[2],
          (*ev_k_buts_KL_sync)[2];

size_t    *buts_BR_prop_iter_sync,
          *buts_KL_prop_iter_sync;

static enum PropSyncAlgo jbu_prop_algo_sync;

static void jacu_sync(int work_step,
                      int temp_kst,
                      int temp_kend);

static void buts_BR_sync(int work_step,
                         int temp_kst,
                         int temp_kend);

static void buts_KL_sync(int work_step,
                         int temp_kst,
                         int temp_kend);

void jacu_buts_init_sync(int iter,
                         int item_default,
                         int blk_size_k,
                         int blk_size)
{
  cl_int ecode;
  int tmp_work_end,
      tmp_work_num_item,
      tmp_work_base,
      tmp_kend, tmp_kst,
      num_k, num_j, num_i,
      num_blk_k,
      num_blk_j,
      num_blk_i,
      kst, kend,
      i;

  double start_t, end_t, KL_time, BR_time;
  size_t jbu_work_num_item;

  k_jbu_datagen_sync = clCreateKernel(p_jacu_buts_sync,
                                      "jacu_buts_datagen_sync",
                                      &ecode);
  clu_CheckError(ecode, "clCreateKernel");

  k_jacu_sync = clCreateKernel(p_jacu_buts_sync,
                               "jacu_sync",
                               &ecode);
  clu_CheckError(ecode, "clCreateKernel");

  k_buts_BR_sync = clCreateKernel(p_jacu_buts_sync,
                                  "buts_BR_sync",
                                  &ecode);
  clu_CheckError(ecode, "clCreateKernel");

  k_buts_KL_sync = clCreateKernel(p_jacu_buts_sync,
                                  "buts_KL_sync",
                                  &ecode);
  clu_CheckError(ecode, "clCreateKernel");

  buts_BR_prop_iter_sync = (size_t*)malloc(sizeof(size_t)*iter);
  buts_KL_prop_iter_sync = (size_t*)malloc(sizeof(size_t)*iter);

  kst = 1; kend = nz-1;

  for (i = 0; i < iter; i++) {
    tmp_work_end = nz - i*item_default;

    if (split_flag) {
      tmp_work_num_item = min(item_default, tmp_work_end);
      tmp_work_base = tmp_work_end - tmp_work_num_item;
      jbu_work_num_item = min(tmp_work_num_item, kend - tmp_work_base);
      tmp_kst = 2 ;

      jbu_work_num_item += min(2, tmp_work_base-1);
      tmp_kst -= min(2, tmp_work_base-1);
      tmp_kend = tmp_kst + jbu_work_num_item;
    }
    else {
      tmp_work_num_item = nz;
      tmp_work_base = 0;

      jbu_work_num_item = kend - kst;
      tmp_kst = kst;
      tmp_kend = kend;
    }

    num_k = tmp_kend - tmp_kst;
    num_j = jend - jst;
    num_i = iend - ist;

    num_blk_k = ((num_k-1)/blk_size_k) + 1;
    num_blk_j = ((num_j-1)/blk_size) + 1;
    num_blk_i = ((num_i-1)/blk_size) + 1;

    buts_BR_prop_iter_sync[i] = num_blk_k + num_blk_j + num_blk_i - 2;
    buts_KL_prop_iter_sync[i] = num_k + num_j +num_i - 2;
  }

  ev_k_jbu_datagen_sync   = (cl_event*)malloc(sizeof(cl_event)*iter);
  ev_k_jacu_sync          = (cl_event*)malloc(sizeof(cl_event)*iter);
  ev_k_buts_BR_sync       = (cl_event(*)[2])malloc(sizeof(cl_event)*iter*2);
  ev_k_buts_KL_sync       = (cl_event(*)[2])malloc(sizeof(cl_event)*iter*2);

  // Propagation Strategy Selection

  // warm up before profiling
  for (i = 0; i < iter; i++) {
    tmp_work_end = nz - i*item_default;

    if (split_flag) {
      tmp_work_num_item = min(item_default, tmp_work_end);
      tmp_work_base = tmp_work_end - tmp_work_num_item;
      jbu_work_num_item = min(tmp_work_num_item, kend - tmp_work_base);
      tmp_kst = 2 ;

      jbu_work_num_item += min(2, tmp_work_base-1);
      tmp_kst -= min(2, tmp_work_base-1);
      tmp_kend = tmp_kst + jbu_work_num_item;
    }
    else {
      tmp_work_num_item = nz;
      tmp_work_base = 0;

      jbu_work_num_item = kend - kst;
      tmp_kst = kst;
      tmp_kend = kend;
    }

    buts_KL_sync(i, tmp_kst, tmp_kend);
    buts_BR_sync(i, tmp_kst, tmp_kend);

    clFinish(cmd_q[KERNEL_Q]);
  }

  // profiling for KL
  timer_clear(t_jbu_KL_prof);
  start_t = timer_read(t_jbu_KL_prof);
  timer_start(t_jbu_KL_prof);

  for (i = 0; i < iter; i++) {
    tmp_work_end = nz - i*item_default;

    if (split_flag) {
      tmp_work_num_item = min(item_default, tmp_work_end);
      tmp_work_base = tmp_work_end - tmp_work_num_item;
      jbu_work_num_item = min(tmp_work_num_item, kend - tmp_work_base);
      tmp_kst = 2 ;

      jbu_work_num_item += min(2, tmp_work_base-1);
      tmp_kst -= min(2, tmp_work_base-1);
      tmp_kend = tmp_kst + jbu_work_num_item;
    }
    else {
      tmp_work_num_item = nz;
      tmp_work_base = 0;

      jbu_work_num_item = kend - kst;
      tmp_kst = kst;
      tmp_kend = kend;
    }

    buts_KL_sync(i, tmp_kst, tmp_kend);

    clFinish(cmd_q[KERNEL_Q]);
  }

  timer_stop(t_jbu_KL_prof);
  end_t = timer_read(t_jbu_KL_prof);
  KL_time = end_t - start_t;

  DETAIL_LOG("KL time : %f", KL_time);

  // profiling for BR
  timer_clear(t_jbu_BR_prof);
  start_t = timer_read(t_jbu_BR_prof);
  timer_start(t_jbu_BR_prof);

  for (i = 0; i < iter; i++) {
    tmp_work_end = nz - i*item_default;

    if (split_flag) {
      tmp_work_num_item = min(item_default, tmp_work_end);
      tmp_work_base = tmp_work_end - tmp_work_num_item;
      jbu_work_num_item = min(tmp_work_num_item, kend - tmp_work_base);
      tmp_kst = 2 ;

      jbu_work_num_item += min(2, tmp_work_base-1);
      tmp_kst -= min(2, tmp_work_base-1);
      tmp_kend = tmp_kst + jbu_work_num_item;
    }
    else {
      tmp_work_num_item = nz;
      tmp_work_base = 0;

      jbu_work_num_item = kend - kst;
      tmp_kst = kst;
      tmp_kend = kend;
    }

    buts_BR_sync(i, tmp_kst, tmp_kend);

    clFinish(cmd_q[KERNEL_Q]);
  }

  timer_stop(t_jbu_BR_prof);
  end_t = timer_read(t_jbu_BR_prof);
  BR_time = end_t - start_t;

  DETAIL_LOG("BR time : %f", BR_time);

  if (KL_time < BR_time)
    jbu_prop_algo_sync = KERNEL_LAUNCH;
  else
    jbu_prop_algo_sync = BARRIER;

  if (jbu_prop_algo_sync == KERNEL_LAUNCH)
    DETAIL_LOG("jacu buts propagation strategy : Kernel Launch");
  else
    DETAIL_LOG("jacu buts propagation strategy : Kernel Launch + Work Group Barrier");
}


void jacu_buts_release_sync(int iter)
{
  clReleaseKernel(k_jbu_datagen_sync);
  clReleaseKernel(k_jacu_sync);
  clReleaseKernel(k_buts_BR_sync);
  clReleaseKernel(k_buts_KL_sync);

  free(ev_k_buts_BR_sync);
  free(ev_k_buts_KL_sync);
  free(ev_k_jacu_sync);
  free(ev_k_jbu_datagen_sync);

  free(buts_BR_prop_iter_sync);
  free(buts_KL_prop_iter_sync);
}

void jacu_buts_release_ev_sync(int iter)
{
  int i;
  for (i = 0; i < iter; i++) {
    if (split_flag) {
      clReleaseEvent(ev_k_jbu_datagen_sync[i]);
    }

    clReleaseEvent(ev_k_jacu_sync[i]);

    if (jbu_prop_algo_sync == BARRIER) {
      clReleaseEvent(ev_k_buts_BR_sync[i][0]);
      if (buts_BR_prop_iter_sync[i] > 1)
        clReleaseEvent(ev_k_buts_BR_sync[i][1]);
    }
    else {
      clReleaseEvent(ev_k_buts_KL_sync[i][0]);
      if (buts_KL_prop_iter_sync[i] > 1)
        clReleaseEvent(ev_k_buts_KL_sync[i][1]);
    }
  }
}

void jacu_buts_body_sync(int work_step,
                         int work_max_iter,
                         int work_num_item,
                         int next_work_num_item,
                         int temp_kst,
                         int temp_kend,
                         cl_event *ev_wb_ptr)
{
  cl_int ecode;
  size_t lws[3], gws[3];

  int num_wait;
  cl_event *wait_ev;
  int buf_idx = (work_step%2)*buffering_flag;

  // ##################
  //  Kernel Execution
  // ##################

  if (split_flag) {

    // recover "u" value before update
    if (work_step > 0) {
      ecode = clEnqueueCopyBuffer(cmd_q[KERNEL_Q], 
                                  m_u_prev, 
                                  m_u[buf_idx],
                                  0, 
                                  u_slice_size*temp_kend,
                                  u_slice_size*2,
                                  0, NULL, 
                                  &loop2_ev_copy_u_prev[work_step]);
      clu_CheckError(ecode, "clEnqueueCopyBuffer ssor2 loop");

      ecode = clEnqueueCopyBuffer(cmd_q[KERNEL_Q], 
                                  m_r_prev, 
                                  m_rsd[buf_idx],
                                  0, 
                                  rsd_slice_size*temp_kend,
                                  rsd_slice_size*2,
                                  0, NULL, 
                                  &loop2_ev_copy_r_prev[work_step]);
      clu_CheckError(ecode, "clEnqueueCopyBuffer ssor2 loop");

      if (buffering_flag)
        clFlush(cmd_q[KERNEL_Q]);
      else
        clFinish(cmd_q[KERNEL_Q]);
    }

    lws[2] = 1;
    lws[1] = 1;
    lws[0] = max_work_group_size;

    gws[2] = temp_kend - temp_kst + 1;
    gws[1] = jend - jst + 1;
    gws[0] = iend - ist + 1;

    gws[2] = clu_RoundWorkSize(gws[2], lws[2]);
    gws[1] = clu_RoundWorkSize(gws[1], lws[1]);
    gws[0] = clu_RoundWorkSize(gws[0], lws[0]);

    ecode  = clSetKernelArg(k_jbu_datagen_sync, 0, sizeof(cl_mem), &m_u[buf_idx]);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 1, sizeof(cl_mem), &m_qs[buf_idx]);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 2, sizeof(cl_mem), &m_rho_i[buf_idx]);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 3, sizeof(int), &jst);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 4, sizeof(int), &jend);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 5, sizeof(int), &ist);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 6, sizeof(int), &iend);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 7, sizeof(int), &temp_kst);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 8, sizeof(int), &temp_kend);
    ecode |= clSetKernelArg(k_jbu_datagen_sync, 9, sizeof(int), &work_num_item);
    clu_CheckError(ecode, "clSetKernelArg");

    num_wait = (buffering_flag) ? 1 : 0;
    wait_ev = (buffering_flag) ? ev_wb_ptr : NULL;

    ecode = clEnqueueNDRangeKernel(cmd_q[KERNEL_Q], 
                                   k_jbu_datagen_sync,
                                   3, NULL, 
                                   gws, lws,
                                   num_wait, wait_ev, 
                                   &ev_k_jbu_datagen_sync[work_step]);
    clu_CheckError(ecode, "clEnqueueNDRangeKerenel");

    if (!buffering_flag)
      clFinish(cmd_q[KERNEL_Q]);
    else
      clFlush(cmd_q[KERNEL_Q]);
  }

  jacu_sync(work_step, temp_kst, temp_kend);

  if (jbu_prop_algo_sync == BARRIER)
    buts_BR_sync(work_step, temp_kst, temp_kend);
  else
    buts_KL_sync(work_step, temp_kst, temp_kend);
 
  if (!buffering_flag) 
    clFinish(cmd_q[KERNEL_Q]);

  if (split_flag) {
    // store temporal data
    if (work_step < work_max_iter - 1) {
      ecode = clEnqueueCopyBuffer(cmd_q[KERNEL_Q], 
                                  m_u[buf_idx], 
                                  m_u_prev,
                                  u_slice_size*2,
                                  0, 
                                  u_slice_size*2,
                                  0, NULL, 
                                  &loop2_ev_copy_u[work_step]);
      clu_CheckError(ecode, "clEnqueueCopyBuffer ssor2 loop");

      if (buffering_flag)
        clFlush(cmd_q[KERNEL_Q]);
      else
        clFinish(cmd_q[KERNEL_Q]);

      ecode = clEnqueueCopyBuffer(cmd_q[KERNEL_Q], 
                                  m_rsd[buf_idx], 
                                  m_r_prev,
                                  rsd_slice_size*2,
                                  0, 
                                  rsd_slice_size*2,
                                  0, NULL, 
                                  &loop2_ev_copy_rsd[work_step]);
      clu_CheckError(ecode, "clEnqueueCopyBuffer ssor2 loop");

      if (buffering_flag)
        clFlush(cmd_q[KERNEL_Q]);
      else
        clFinish(cmd_q[KERNEL_Q]);
    }
  }
}

static void jacu_sync(int work_step,
                      int temp_kst,
                      int temp_kend)
{

  size_t gws[3], lws[3];
  int buf_idx = (work_step%2)*buffering_flag;
  cl_int ecode;
  cl_event *my_ev = NULL;

  gws[2] = temp_kend - temp_kst;
  gws[1] = jend - jst;
  gws[0] = iend - ist;

  lws[2] = 1;
  lws[1] = 1;
  lws[0] = max_work_group_size;

  gws[2] = clu_RoundWorkSize(gws[2], lws[2]);
  gws[1] = clu_RoundWorkSize(gws[1], lws[1]);
  gws[0] = clu_RoundWorkSize(gws[0], lws[0]);

  ecode  = clSetKernelArg(k_jacu_sync, 0, sizeof(cl_mem), &m_rsd[buf_idx]);
  ecode |= clSetKernelArg(k_jacu_sync, 1, sizeof(cl_mem), &m_u[buf_idx]);
  ecode |= clSetKernelArg(k_jacu_sync, 2, sizeof(cl_mem), &m_qs[buf_idx]);
  ecode |= clSetKernelArg(k_jacu_sync, 3, sizeof(cl_mem), &m_rho_i[buf_idx]);
  ecode |= clSetKernelArg(k_jacu_sync, 4, sizeof(cl_mem), &m_a);
  ecode |= clSetKernelArg(k_jacu_sync, 5, sizeof(cl_mem), &m_b);
  ecode |= clSetKernelArg(k_jacu_sync, 6, sizeof(cl_mem), &m_c);
  ecode |= clSetKernelArg(k_jacu_sync, 7, sizeof(cl_mem), &m_d);
  ecode |= clSetKernelArg(k_jacu_sync, 8, sizeof(int), &nz);
  ecode |= clSetKernelArg(k_jacu_sync, 9, sizeof(int), &ny);
  ecode |= clSetKernelArg(k_jacu_sync, 10, sizeof(int), &nx);
  ecode |= clSetKernelArg(k_jacu_sync, 11, sizeof(int), &jst);
  ecode |= clSetKernelArg(k_jacu_sync, 12, sizeof(int), &jend);
  ecode |= clSetKernelArg(k_jacu_sync, 13, sizeof(int), &ist);
  ecode |= clSetKernelArg(k_jacu_sync, 14, sizeof(int), &iend);
  ecode |= clSetKernelArg(k_jacu_sync, 15, sizeof(int), &temp_kst);
  ecode |= clSetKernelArg(k_jacu_sync, 16, sizeof(int), &temp_kend);
  clu_CheckError(ecode, "clSetKernelArg()");

  my_ev = &ev_k_jacu_sync[work_step];

  ecode = clEnqueueNDRangeKernel(cmd_q[KERNEL_Q], 
                                 k_jacu_sync, 
                                 3, NULL,
                                 gws, lws,
                                 0, NULL, 
                                 my_ev);
  clu_CheckError(ecode, "clEnqueueNDRangeKernel()");

}

static void buts_BR_sync(int work_step,
                         int temp_kst,
                         int temp_kend)
{

  cl_int ecode;
  size_t lws[3], gws[3];
  int num_k, num_j, num_i,
      num_block_k, num_block_j, num_block_i,
      wg00_tail_k, wg00_tail_j, wg00_tail_i,
      wg00_block_k, wg00_block_j, wg00_block_i;
  int iter, diagonals, depth, num_wg, max_iter;
  int buf_idx = (work_step%2)*buffering_flag;
  cl_event *my_ev = NULL;

  wg00_tail_k = temp_kend-1;
  wg00_tail_j = jend-1;
  wg00_tail_i = iend-1;

  num_k = temp_kend - temp_kst;
  num_j = jend - jst;
  num_i = iend - ist;

  num_block_k = ( ( num_k - 1) / block_size_k ) + 1;
  num_block_j = ( ( num_j - 1) / block_size ) + 1;
  num_block_i = ( ( num_i - 1) / block_size ) + 1;

  max_iter = num_block_k + num_block_j + num_block_i - 2;

  //blocking iteration
  for (iter = 0; iter < max_iter; iter++) {

    wg00_block_k = (temp_kend-1 - wg00_tail_k) / block_size_k;
    wg00_block_j = (jend-1 - wg00_tail_j) / block_size;
    wg00_block_i = (iend-1 - wg00_tail_i) / block_size;

    //CAUTION !!!!!
    //block indexing direction and element indexing(in one block) 
    //direction is opposite!!!
    num_wg = 0;

    // diagonals = the number of active diagonals
    diagonals = min(wg00_block_k+1, num_block_j+num_block_i-1 - (wg00_block_j+wg00_block_i));

    for (depth = 0 ; depth < diagonals; depth++) {
      num_wg += min(wg00_block_j+1, num_block_i - wg00_block_i);

      wg00_block_j++;
      if (wg00_block_j >= num_block_j) {
        wg00_block_j--;
        wg00_block_i++;
      }
    }

    wg00_block_k = (temp_kend-1 - wg00_tail_k) / block_size_k;
    wg00_block_j = (jend-1 - wg00_tail_j) / block_size;
    wg00_block_i = (iend-1 - wg00_tail_i) / block_size;

    lws[1] = (size_t)block_size;
    lws[0] = (size_t)block_size;

    gws[1] = (size_t)block_size*num_wg;
    gws[0] = (size_t)block_size;

    lws[0] = jacu_buts_lws;
    gws[0] = lws[0]*num_wg;

    gws[1] = clu_RoundWorkSize(gws[1], lws[1]);
    gws[0] = clu_RoundWorkSize(gws[0], lws[0]);

    ecode  = clSetKernelArg(k_buts_BR_sync, 0, sizeof(cl_mem), &m_rsd[buf_idx]);
    ecode |= clSetKernelArg(k_buts_BR_sync, 1, sizeof(cl_mem), &m_rho_i[buf_idx]);
    ecode |= clSetKernelArg(k_buts_BR_sync, 2, sizeof(cl_mem), &m_u[buf_idx]);
    ecode |= clSetKernelArg(k_buts_BR_sync, 3, sizeof(cl_mem), &m_qs[buf_idx]);
    ecode |= clSetKernelArg(k_buts_BR_sync, 4, sizeof(cl_mem), &m_a);
    ecode |= clSetKernelArg(k_buts_BR_sync, 5, sizeof(cl_mem), &m_b);
    ecode |= clSetKernelArg(k_buts_BR_sync, 6, sizeof(cl_mem), &m_c);
    ecode |= clSetKernelArg(k_buts_BR_sync, 7, sizeof(cl_mem), &m_d);
    ecode |= clSetKernelArg(k_buts_BR_sync, 8, sizeof(int), &temp_kst);
    ecode |= clSetKernelArg(k_buts_BR_sync, 9, sizeof(int), &temp_kend);
    ecode |= clSetKernelArg(k_buts_BR_sync, 10, sizeof(int), &jst);
    ecode |= clSetKernelArg(k_buts_BR_sync, 11, sizeof(int), &jend);
    ecode |= clSetKernelArg(k_buts_BR_sync, 12, sizeof(int), &ist);
    ecode |= clSetKernelArg(k_buts_BR_sync, 13, sizeof(int), &iend);
    ecode |= clSetKernelArg(k_buts_BR_sync, 14, sizeof(int), &wg00_tail_k);
    ecode |= clSetKernelArg(k_buts_BR_sync, 15, sizeof(int), &wg00_tail_j);
    ecode |= clSetKernelArg(k_buts_BR_sync, 16, sizeof(int), &wg00_tail_i);
    ecode |= clSetKernelArg(k_buts_BR_sync, 17, sizeof(int), &wg00_block_k);
    ecode |= clSetKernelArg(k_buts_BR_sync, 18, sizeof(int), &wg00_block_j);
    ecode |= clSetKernelArg(k_buts_BR_sync, 19, sizeof(int), &wg00_block_i);
    ecode |= clSetKernelArg(k_buts_BR_sync, 20, sizeof(int), &num_block_k);
    ecode |= clSetKernelArg(k_buts_BR_sync, 21, sizeof(int), &num_block_j);
    ecode |= clSetKernelArg(k_buts_BR_sync, 22, sizeof(int), &num_block_i);
    ecode |= clSetKernelArg(k_buts_BR_sync, 23, sizeof(int), &block_size);
    ecode |= clSetKernelArg(k_buts_BR_sync, 24, sizeof(int), &block_size_k);
    clu_CheckError(ecode, "clSetKernelArg");

    if (iter == 0)
      my_ev = &ev_k_buts_BR_sync[work_step][0];
    else if (iter == max_iter-1)
      my_ev = &ev_k_buts_BR_sync[work_step][1];
    else
      my_ev = NULL;

    ecode = clEnqueueNDRangeKernel(cmd_q[KERNEL_Q], 
                                   k_buts_BR_sync,
                                   1, NULL, 
                                   gws, lws,
                                   0, NULL, 
                                   my_ev);
    clu_CheckError(ecode , "clEnqueueNDRangeKerenel");

    if (buffering_flag)
      clFlush(cmd_q[KERNEL_Q]);

    wg00_tail_k -= block_size_k;
    if (wg00_tail_k < temp_kst) {
      wg00_tail_k += block_size_k;
      wg00_tail_j -= block_size;
      if (wg00_tail_j < jst) {
        wg00_tail_j += block_size;
        wg00_tail_i -= block_size;
      }
    }

  }
}

static void buts_KL_sync(int work_step,
                         int temp_kst,
                         int temp_kend)
{

  int lbk, ubk, lbj, ubj, temp, k;
  size_t buts_max_work_group_size = 64;
  size_t buts_max_work_item_sizes0 = 64;
  size_t buts_gws[3], buts_lws[3];
  int buf_idx = (work_step%2)*buffering_flag;
  cl_int ecode;
  cl_event *my_ev = NULL;

  for (k = (temp_kend-temp_kst-1)+(iend-ist-1)+(jend-jst-1); k >= 0; k--) {
    lbk = (k-(iend-ist-1)-(jend-jst-1)) >= 0 ? (k-(iend-ist-1)-(jend-jst-1)) : 0;
    ubk = k < (temp_kend-temp_kst-1) ? k : (temp_kend-temp_kst);
    lbj = (k-(iend-ist-1)-(temp_kend-temp_kst)) >= 0 ? (k-(iend-ist-1)-(temp_kend-temp_kst)) : 0;
    ubj = k < (jend-jst-1) ? k : (jend-jst-1);

    ecode  = clSetKernelArg(k_buts_KL_sync, 0, sizeof(cl_mem), &m_rsd[buf_idx]);
    ecode |= clSetKernelArg(k_buts_KL_sync, 1, sizeof(cl_mem), &m_u[buf_idx]);
    ecode |= clSetKernelArg(k_buts_KL_sync, 2, sizeof(cl_mem), &m_qs[buf_idx]);
    ecode |= clSetKernelArg(k_buts_KL_sync, 3, sizeof(cl_mem), &m_rho_i[buf_idx]);
    ecode |= clSetKernelArg(k_buts_KL_sync, 4, sizeof(cl_mem), &m_a);
    ecode |= clSetKernelArg(k_buts_KL_sync, 5, sizeof(cl_mem), &m_b);
    ecode |= clSetKernelArg(k_buts_KL_sync, 6, sizeof(cl_mem), &m_c);
    ecode |= clSetKernelArg(k_buts_KL_sync, 7, sizeof(cl_mem), &m_d);
    ecode |= clSetKernelArg(k_buts_KL_sync, 8, sizeof(int), &nz);
    ecode |= clSetKernelArg(k_buts_KL_sync, 9, sizeof(int), &ny);
    ecode |= clSetKernelArg(k_buts_KL_sync, 10, sizeof(int), &nx);
    ecode |= clSetKernelArg(k_buts_KL_sync, 11, sizeof(int), &k);
    ecode |= clSetKernelArg(k_buts_KL_sync, 12, sizeof(int), &lbk);
    ecode |= clSetKernelArg(k_buts_KL_sync, 13, sizeof(int), &lbj);
    ecode |= clSetKernelArg(k_buts_KL_sync, 14, sizeof(int), &jst);
    ecode |= clSetKernelArg(k_buts_KL_sync, 15, sizeof(int), &jend);
    ecode |= clSetKernelArg(k_buts_KL_sync, 16, sizeof(int), &ist);
    ecode |= clSetKernelArg(k_buts_KL_sync, 17, sizeof(int), &iend);
    ecode |= clSetKernelArg(k_buts_KL_sync, 18, sizeof(int), &temp_kst);
    ecode |= clSetKernelArg(k_buts_KL_sync, 19, sizeof(int), &temp_kend);
    clu_CheckError(ecode, "clSetKernelArg()");

    buts_lws[0] = (ubj-lbj+1) < buts_max_work_item_sizes0 ? (ubj-lbj+1) : buts_max_work_item_sizes0;
    temp = buts_max_work_group_size / buts_lws[0];
    buts_lws[1] = (ubk-lbk+1) < temp ? (ubk-lbk+1) : temp;
    buts_gws[0] = clu_RoundWorkSize((size_t)(ubj-lbj+1), buts_lws[0]);
    buts_gws[1] = clu_RoundWorkSize((size_t)(ubk-lbk+1), buts_lws[1]);


    if (k == (temp_kend-temp_kst-1)+(iend-ist-1)+(jend-jst-1))
      my_ev = &ev_k_buts_KL_sync[work_step][0];
    else if (k == 0)
      my_ev = &ev_k_buts_KL_sync[work_step][1];
    else
      my_ev = NULL;

    ecode = clEnqueueNDRangeKernel(cmd_q[KERNEL_Q], 
                                   k_buts_KL_sync, 
                                   2, NULL,
                                   buts_gws, buts_lws,
                                   0, NULL, 
                                   my_ev);
    clu_CheckError(ecode, "clEnqueueNDRangeKernel()");

    if (buffering_flag)
      clFlush(cmd_q[KERNEL_Q]);

  }
}

