//-------------------------------------------------------------------------//
//                                                                         //
//  This benchmark is an CUDA® C version of the NPB LU code. This CUDA® C  //
//  version is a part of SNU-NPB 2019 developed by the Center for Manycore //
//  Programming at Seoul National University and derived from the serial   //
//  Fortran versions in "NPB3.3.1-SER" developed by NAS.                   //
//                                                                         //
//  Permission to use, copy, distribute and modify this software for any   //
//  purpose with or without fee is hereby granted. This software is        //
//  provided "as is" without express or implied warranty.                  //
//                                                                         //
//  Information on original NPB 3.3.1, including the technical report, the //
//  original specifications, source code, results and information on how   //
//  to submit new results, is available at:                                //
//                                                                         //
//           http://www.nas.nasa.gov/Software/NPB/                         //
//                                                                         //
//  Information on SNU-NPB 2019, including the conference paper and source //
//  code, is available at:                                                 //
//                                                                         //
//           http://aces.snu.ac.kr                                         //
//                                                                         //
//  Send comments or suggestions for this CUDA® C version to               //
//  snunpb@aces.snu.ac.kr                                                  //
//                                                                         //
//          Center for Manycore Programming                                //
//          School of Computer Science and Engineering                     //
//          Seoul National University                                      //
//          Seoul 08826, Korea                                             //
//                                                                         //
//          E-mail: snunpb@aces.snu.ac.kr                                  //
//                                                                         //
//-------------------------------------------------------------------------//

//-------------------------------------------------------------------------//
// Authors: Youngdong Do, Hyung Mo Kim, Pyeongseok Oh, Daeyoung Park,      //
//          and Jaejin Lee                                                 //
//-------------------------------------------------------------------------//

#include "applu.incl"

//---------------------------------------------------------------------
//
// set the initial values of independent variables based on tri-linear
// interpolation of boundary values in the computational space.
//
//---------------------------------------------------------------------
void setiv()
{
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  int i, j, k, m;
  double xi, eta, zeta;
  double pxi, peta, pzeta;
  double ue_1jk[5], ue_nx0jk[5], ue_i1k[5];
  double ue_iny0k[5], ue_ij1[5], ue_ijnz[5];

  for (k = 1; k < nz - 1; k++) {
    zeta = ( (double)k ) / (nz-1);
    for (j = 1; j < ny - 1; j++) {
      eta = ( (double)j ) / (ny0-1);
      for (i = 1; i < nx - 1; i++) {
        xi = ( (double)i ) / (nx0-1);
        exact(0, j, k, ue_1jk);
        exact(nx0-1, j, k, ue_nx0jk);
        exact(i, 0, k, ue_i1k);
        exact(i, ny0-1, k, ue_iny0k);
        exact(i, j, 0, ue_ij1);
        exact(i, j, nz-1, ue_ijnz);

        for (m = 0; m < 5; m++) {
          pxi =   ( 1.0 - xi ) * ue_1jk[m]
                        + xi   * ue_nx0jk[m];
          peta =  ( 1.0 - eta ) * ue_i1k[m]
                        + eta   * ue_iny0k[m];
          pzeta = ( 1.0 - zeta ) * ue_ij1[m]
                        + zeta   * ue_ijnz[m];

          u[k][j][i][m] = pxi + peta + pzeta
            - pxi * peta - peta * pzeta - pzeta * pxi
            + pxi * peta * pzeta;
        }
      }
    }
  }
}

