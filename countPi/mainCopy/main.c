#include <stdlib.h>                                                             
#include <stdio.h>                                                              
#include <NIDAQmx.h>                                                            
                                                                                
//Includes for writing to temp files                                            
#include<unistd.h>                                                              
#include<string.h>                                                              
#include<errno.h>
#include<math.h>

//--- Start of setup from main.c ---
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
                                                                                
// This is the name of the DAQ device                                           
#define DAQ_DEVICE_NAME "cDAQ1"                                                 
                                                                                
// This is the name of the module port we measure from                          
#define DAQ_MODULE_NAME "Mod3"                                                  
#define DAQ_DEVICE DAQ_DEVICE_NAME DAQ_MODULE_NAME                              
                                                                                
// The END channel is INCLUSIVE                                                 
// These are the pin indices used for DIFFERENTIAL mode                         
// Pin X will automatically be paired with its corresponding                    
// x+8 pin.                                                                     
// This setup assumes we are using pins 0, 1, 2, and 3                          
// with their complementary pins 8, 9, 10, and 11                               
// which are automatically read by the NIDAQ library                            
// when we are in differential mode                                             
#define DIFF_INPUTS_BEGIN "0"                                                   
#define DIFF_INPUTS_END   "3"                                                   
                                                                                
// We'll pass this into the CreateAIVoltageChan to specify                      
// the channels we want to work with.                                           
#define DIFF_CHANNELS DAQ_DEVICE "/ai" DIFF_INPUTS_BEGIN ":" DIFF_INPUTS_END    
#define PHYS_CHANNELS DIFF_CHANNELS                                             
                                                                                
// The name assigned to our task                                                
#define DAQ_TASK_NAME ""                                                        
                                                                                
// The name we assign to our voltage channel                                    
#define CHANNEL_NAME ""                                                         
                                                                                
// The minimum and maximum volts we expect to measure                           
// The NIDAQ device only supports max of -10/10 volts                           
// This is not good if we have a 12V supply we want to measure                  
#define MIN_VOLTS -10.0                                                         
#define MAX_VOLTS  10.0                                                         
                                                                                
// Number of samples to collect each second for each channel                    
//#define SAMPLES_PER_SEC 1000                                                  
                                                                                
// The number of samples we want to take for each channel                       
//#define SAMPLES_PER_CHANNEL 16000                                             
//#define SAMPLES_PER_CHANNEL 30000                                             
                                                                                
// The amount of time to wait to read the samples                               
#define SAMPLES_WAIT_TIMEOUT_SECS 100                                           
                                                                                
// The number of differential channel pairs we will read from                   
//#define NUM_CHANNEL_PAIRS 4                                                   
#define NUM_CHANNEL_PAIRS 4                                                     
                                                                                
// The number of samples we expect to collect                                   
#define ARRAY_SIZE_IN_SAMPLES NUM_CHANNEL_PAIRS*SAMPLES_PER_CHANNEL             
                                                                                
// The resistance of the resister that we measure the voltage diff over         
#define RESISTOR_OHMS 0.003                                                     
                                                                                
// The voltage we assume that all the lines are running at                      
#define LINE_VOLTAGE 12                 
//--- End of setup from main.c --- 

//int main()
int main(int argc, char *argv[])
{

  //--- Start of values from main.c ---
  int SAMPLES_PER_SEC = atoi(argv[1]);                                          
  int SAMPLING_SECS = atoi(argv[2]);                                            
  int SAMPLES_PER_CHANNEL = SAMPLING_SECS*SAMPLES_PER_SEC;                      
                                                                                
  //-- open input file --                                                       
  char str[] = "----- Test String -----";                                       
  FILE *fp = tmpfile();                                                         
  if (fp == NULL) {                                                             
    puts("unable to create fp file");                                           
  }                                                                             
                                                                                
  int32       error = 0;                                                        
  TaskHandle  taskHandle = 0;                                                   
  int32       samples_read_per_channel;                                         
  float64     data[ARRAY_SIZE_IN_SAMPLES];                                      
  char        errBuff[2048]={'\0'};
  //--- End of values from main.c -`--

 long int i, n;
 double sum=0.0, term, pi;
 
 printf("Enter number of terms: ");
 scanf("%ld", &n);
 //n = atoi(argv[1]);

  createTask();                                                                 
  createAIVoltageChan();                                                        
  createAIVoltageChan();                                                        
  setSampleClockAndRate();                                                      
  startTask();                                                                  
  takeSamples();                                                                

 /* Applying Leibniz Formula */
 for(i=0;i< n;i++)
 {
  term = pow(-1, i) / (2*i+1);
  sum += term;
 }
 pi = 4 * sum;

  finalize();

 printf("\nPI = %.6lf", pi);
 
 return 0;
}
