//---------------------------------------------------------------------------//
//                                                                           //
//  This benchmark is an OpenCL C version of the NPB BT code. This OpenCL C  //
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

__kernel void add(__global double *m_u, 
                  __global double *m_rhs,
                  int gp0, int gp1, int gp2,
                  int work_base, 
                  int work_num_item, 
                  int split_flag)
{
  int k = get_global_id(2)+1;
  int j = get_global_id(1);
  int t_i = get_global_id(0);
  int i = t_i/5 + 1;
  int m = t_i%5;

  if (k > gp2-2 || j+work_base < 1 || j+work_base > gp1-2 || j >= work_num_item || i > gp0-2) return;

  if (!split_flag) j += work_base;

  __global double (* u)[WORK_NUM_ITEM_DEFAULT_J][IMAXP+1][5]
    = (__global double (*) [WORK_NUM_ITEM_DEFAULT_J][IMAXP+1][5])m_u;
  __global double (* rhs)[WORK_NUM_ITEM_DEFAULT_J][IMAXP+1][5]
    = (__global double (*) [WORK_NUM_ITEM_DEFAULT_J][IMAXP+1][5])m_rhs;

  u[k][j][i][m] = u[k][j][i][m] + rhs[k][j][i][m];
}

