/*****************************************************************/
/******     C  _  P  R  I  N  T  _  R  E  S  U  L  T  S     ******/
/*****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void c_print_results( char  *name,
                      char   class,
                      int    n1, 
                      int    n2,
                      int    n3,
                      int    niter,
                      double t,
                      double mops,
		      char  *optype,
                      int    passed_verification,
                      char  *npbversion,
                      char  *compiletime,
                      char  *cc,
                      char  *clink,
                      char  *c_lib,
                      char  *c_inc,
                      char  *cflags,
                      char  *clinkflags,
                      char  *crand,
                const char  *cuda_dev_name )
{
  char size[16];
  int j;

  printf( "\n\n %s Benchmark Completed\n", name ); 
  printf( " Class           =                        %c\n", class );

  if ( ( n2 == 0 ) && ( n3 == 0 ) ) {
    if ( ( name[0] == 'E' ) && ( name[1] == 'P' ) ) {
      sprintf( size, "%15.0lf", pow(2.0, n1) );
      j = 14;
      if ( size[j] == '.' ) {
        size[j] = ' '; 
        j--;
      }
      size[j+1] = '\0';
      printf( " Size            =          %15s\n", size );
    } else {
      printf( " Size            =             %12d\n", n1 );
    }
  } else if( n3 == 0 ) {
    long nn = n1;
    if ( n2 != 0 ) nn *= n2;
    printf( " Size            =             %12ld\n", nn );   /* as in IS */
  }
  else
    printf( " Size            =           %4dx%4dx%4d\n", n1,n2,n3 );

  printf( " Iterations      =             %12d\n", niter );
  printf( " Time in seconds =             %12.9f\n", t );
  printf( " Mop/s total     =             %12.2f\n", mops );
  printf( " Operation type  = %24s\n", optype);

  if( passed_verification < 0 )
    printf( " Verification    =            NOT PERFORMED\n" );
  else if( passed_verification )
    printf( " Verification    =               SUCCESSFUL\n" );
  else
    printf( " Verification    =             UNSUCCESSFUL\n" );

  printf( " Version         =             %12s\n", npbversion );
  printf( " Compile date    =             %12s\n", compiletime );

  printf( "\n Compile options:\n" );
  printf( "    CC           = %s\n", cc );
  printf( "    CLINK        = %s\n", clink );
  printf( "    C_LIB        = %s\n", c_lib );
  printf( "    C_INC        = %s\n", c_inc );
  printf( "    CFLAGS       = %s\n", cflags );
  printf( "    CLINKFLAGS   = %s\n", clinkflags );

  printf("\n CUDA options:\n" );
  printf("    Device name  = %s\n", cuda_dev_name );

  printf( "\n--------------------------------------\n");
  printf( " Please send all errors/feedbacks to:\n");
  printf( " Center for Manycore Programming\n");
  printf( " cmp@aces.snu.ac.kr\n");
  printf( " http://aces.snu.ac.kr\n");
  printf( "--------------------------------------\n");
}
