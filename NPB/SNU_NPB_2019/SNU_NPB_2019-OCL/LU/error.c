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
#include <math.h>
#include "applu.incl"

//---------------------------------------------------------------------
// 
// compute the solution error
// 
//---------------------------------------------------------------------
void error()
{
  //---------------------------------------------------------------------
  // local variables
  //---------------------------------------------------------------------
  int i, j, k, m;
  double tmp;
  double u000ijk[5];

  for (m = 0; m < 5; m++) {
    errnm[m] = 0.0;
  }

  for (k = 1; k < nz-1; k++) {
    for (j = jst; j < jend; j++) {
      for (i = ist; i < iend; i++) {
        exact( i, j, k, u000ijk );
        for (m = 0; m < 5; m++) {
          tmp = ( u000ijk[m] - u[k][j][i][m] );
          errnm[m] = errnm[m] + tmp * tmp;
        }
      }
    }
  }


  for (m = 0; m < 5; m++) {
    errnm[m] = sqrt ( errnm[m] / ( (nx0-2)*(ny0-2)*(nz0-2) ) );
  }

  /*
  printf(" \n RMS-norm of error in soln. to first pde  = %12.5E\n"
         " RMS-norm of error in soln. to second pde = %12.5E\n"
         " RMS-norm of error in soln. to third pde  = %12.5E\n"
         " RMS-norm of error in soln. to fourth pde = %12.5E\n"
         " RMS-norm of error in soln. to fifth pde  = %12.5E\n",
         errnm[0], errnm[1], errnm[2], errnm[3], errnm[4]);
  */
}

