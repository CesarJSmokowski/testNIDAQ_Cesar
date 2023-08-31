//---------------------------------------------------------------------------//
//                                                                           //
//  This benchmark is an OpenCL C version of the NPB SP code. This OpenCL C  //
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

#include "npbparams.h"
#include "type.h"

// If processor array is 1x1 -> 0D grid decomposition

// Cache blocking params. These values are good for most
// RISC processors.  
// FFT parameters:
//  fftblock controls how many ffts are done at a time. 
//  The default is appropriate for most cache-based machines
//  On vector machines, the FFT can be vectorized with vector
//  length equal to the block size, so the block size should
//  be as large as possible. This is the size of the smallest
//  dimension of the problem: 128 for class A, 256 for class B and
//  512 for class C.

#define FFTBLOCK_DEFAULT      32
#define FFTBLOCKPAD_DEFAULT   33

/* common /blockinfo/ */
//static int fftblock, fftblockpad;

// we need a bunch of logic to keep track of how
// arrays are laid out. 


// Note: this serial version is the derived from the parallel 0D case
// of the ft NPB.
// The computation proceeds logically as

// set up initial conditions
// fftx(1)
// transpose (1->2)
// ffty(2)
// transpose (2->3)
// fftz(3)
// time evolution
// fftz(3)
// transpose (3->2)
// ffty(2)
// transpose (2->1)
// fftx(1)
// compute residual(1)

// for the 0D, 1D, 2D strategies, the layouts look like xxx
//        
//            0D        1D        2D
// 1:        xyz       xyz       xyz

// the array dimensions are stored in dims(coord, phase)
/* common /layout/ */
static int dims[3];

enum {
  T_total,
  T_setup,

  T_compute_im,
  T_compute_im_kern,
  T_compute_im_comm,

  T_compute_ics,
  T_compute_ics_kern,
  T_compute_ics_comm,

  T_fft_init,

  T_evolve,
  T_evolve_kern,
  T_evolve_comm,

  T_fft,

  T_fft_x_kern,
  T_fft_y_kern,
  T_fft_xy_comm,

  T_fft_z_kern,
  T_fft_z_comm,

  T_checksum,
  T_checksum_kern,
  T_checksum_comm,
  T_checksum_host,

  T_max
};

// other stuff
/* common /dbg/ */
static logical timers_enabled;
static logical debug;

#define SEED          314159265.0
#define A             1220703125.0
#define PI            3.141592653589793238
#define ALPHA         1.0e-6


// roots of unity array
// relies on x being largest dimension?
/* common /ucomm/ */
static dcomplex u[NXP];


// for checksum data
/* common /sumcomm/ */
static dcomplex sums[NITER_DEFAULT+1];

// number of iterations
/* common /iter/ */
static int niter;


#define dcmplx(r,i)       (dcomplex){r, i}
#define dcmplx_add(a,b)   (dcomplex){(a).real+(b).real, (a).imag+(b).imag}
#define dcmplx_sub(a,b)   (dcomplex){(a).real-(b).real, (a).imag-(b).imag}
#define dcmplx_mul(a,b)   (dcomplex){((a).real*(b).real)-((a).imag*(b).imag),\
                                     ((a).real*(b).imag)+((a).imag*(b).real)}
#define dcmplx_mul2(a,b)  (dcomplex){(a).real*(b), (a).imag*(b)}
static inline dcomplex dcmplx_div(dcomplex z1, dcomplex z2) {
  double a = z1.real;
  double b = z1.imag;
  double c = z2.real;
  double d = z2.imag;

  double divisor = c*c + d*d;
  double real = (a*c + b*d) / divisor;
  double imag = (b*c - a*d) / divisor;
  dcomplex result = (dcomplex){real, imag};
  return result;
}
#define dcmplx_div2(a,b)  (dcomplex){(a).real/(b), (a).imag/(b)}
#define dcmplx_abs(x)     sqrt(((x).real*(x).real) + ((x).imag*(x).imag))

#define dconjg(x)         (dcomplex){(x).real, -1.0*(x).imag}

