#include <stdlib.h>                               
#include <stdio.h>                                       
#include <NIDAQmx.h>                                                    
#include<math.h>

//--- Start of setup from main.c ---
//--- End of setup from main.c --- 

int main()
//int main(int argc, char *argv[])
{

  //--- Start of values from main.c ---
  //--- End of values from main.c -`--

 long int i, n;
 double sum=0.0, term, pi;
 
 //printf("Enter number of terms: ");
 //scanf("%ld", &n);
 //n = atoi(argv[1]);
 n = 1000000;

  createTask();                                                                 
  createAIVoltageChan();                                                        
  createAIVoltageChan();                                                        
  setSampleClockAndRate();                                                      
  startTask();

  //addToDataArray();
                                                                  
  takeSamples();                                                                

 /* Applying Leibniz Formula */
 for(i=0;i< n;i++)
 {
  term = pow(-1, i) / (2*i+1);
  sum += term;
  //addToDataArray();
 }
 pi = 4 * sum;

  finalize();

 printf("\nPI = %.6lf", pi);
 
 return 0;
}
