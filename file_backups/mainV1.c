
#include <stdlib.h>
#include <stdio.h>
#include <NIDAQmx.h>

//Includes for writing to temp files
#include<unistd.h>
#include<string.h>
#include<errno.h>

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


int main(int argc, char** argv){

//-------- Beginning of Writing to Temporary File Example -----

//-- open input file --
  FILE *tempfp;
  tempfp = fopen("tempresults.txt", "w");

// buffer to hold the temporary file name
    char nameBuff[32];
    // buffer to hold data to be written/read to/from temporary file
    //char buffer[24];
    char buffer[100000];
    int filedes = -1,count=0;

    // memset the buffers to 0
    memset(nameBuff,0,sizeof(nameBuff));
    memset(buffer,0,sizeof(buffer));

    // Copy the relevant information in the buffers
    strncpy(nameBuff,"/tmp/myTmpFile-XXXXXX",21);
    //strncpy(buffer,"Hello World",11);

//-----> Start of Original Main.c code <-----

if(argc != 4){
    strncpy(buffer, "Invalid input argument count", 28); 
    strcpy(buffer, "Usage: ./main [SAMPLES_PER_SECOND] [SAMPLING_TIME_IN_SECONDS]\n"); //61 or 62  
    exit(-1);
  }

  int SAMPLES_PER_SEC = atoi(argv[1]);
  int SAMPLING_SECS = atoi(argv[2]);
  int SAMPLES_PER_CHANNEL = SAMPLING_SECS*SAMPLES_PER_SEC;

  //-- open input file --
  FILE *fp;
  //fp = fopen(argv[3], "w");
  fp = fopen("tempMainOutput.txt", "w");

  int32       error = 0;
  TaskHandle  taskHandle = 0;
  int32       samples_read_per_channel;
  float64     data[ARRAY_SIZE_IN_SAMPLES];
  char        errBuff[2048]={'\0'};

  //---- Printing output to CSV in temp -----
  //FILE *fpt;
  //fpt = fopen("Main_Output.csv", "w+");



 //---- Printing output to CSV in temp -----

  //printf("Starting task creation!\n");

  DAQmxErrChk (DAQmxCreateTask(DAQ_TASK_NAME, &taskHandle));

  //printf("Task created!\n");
  //Start in differential mode
  DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle,
                                       PHYS_CHANNELS,
                                       //"cDAQ1Mod3/ai0:3, cDAQ1Mod3/ai8:11",   
                                       CHANNEL_NAME,
                                       DAQmx_Val_Diff,
                                       //DAQmx_Val_NRSE,
                                       //DAQmx_Val_RSE,
                                       MIN_VOLTS, MAX_VOLTS,
                                       DAQmx_Val_Volts, NULL));

  //printf("Voltage Chan created!\n");
  // Setup the sample clock and the rate at which we collect samples            
  DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, NULL, SAMPLES_PER_SEC,          
                                    DAQmx_Val_Rising,                           
                                    DAQmx_Val_FiniteSamps,                      
                                    SAMPLES_PER_CHANNEL ));                     
                                                                                
  //printf("Sampling rate set!\n");                                             
                                                                                
  // DAQmx Start Code                                                           
  DAQmxErrChk(DAQmxStartTask(taskHandle));                                      
                                                                                
  //printf("Task started!\n");                                                  
                                                                                
  // DAQmx Read Code -- i.e: take samples                                       
  // The samples are written interleaved with the GroupByScanNumber             
  DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, SAMPLES_PER_CHANNEL,               
                                 SAMPLES_WAIT_TIMEOUT_SECS,                     
                                 DAQmx_Val_GroupByScanNumber,                   
                                 //DAQmx_Val_GroupByChannel,                    
                                 data, ARRAY_SIZE_IN_SAMPLES,                   
                                 &samples_read_per_channel,                     
                                 NULL));                                        
                                                                                
  // DAQmx Stop and clear task
Error:                                                                          
  if( DAQmxFailed(error) ){                                                     
    DAQmxGetExtendedErrorInfo(errBuff,2048);                                    
  }                                                                             
  if( taskHandle != 0 )  {                                                      
    //printf("NIDAQ testing complete!\n");                                      
                                                                                
    DAQmxStopTask(taskHandle);                                                  
    DAQmxClearTask(taskHandle);                                                 
                                                                                
    //printf("We read [%d] samples for each channel\n", samples_read_per_channel);
    //printf("We took [%d] samples per second\n", SAMPLES_PER_SEC);             
                                                                                
    // Print out the data we collected on differences across the paired pins       
                                                                                
    float64 power;                                                              
    float64 time;                                                               
    int i,j;                                                                    
                                                                                
    // Print the header of the csv                                              
    //printf("time, ");                                                         
    fprintf(fp, "time, ");                                                      
    for(i = 0; i < NUM_CHANNEL_PAIRS-1; i++){                                   
      //printf("line%d, ",i);                                                   
      strcpy(buffer, "line%d, ",i);                                                
    }                                                                           
    //printf("line%d\n",i);                                                     
    strcpy(buffer, "line%d\n",i);                                                  
                                                                                
    // Print the data                                                           
    for(i = 0; i < ARRAY_SIZE_IN_SAMPLES; i+=NUM_CHANNEL_PAIRS){                
      time = (float)i/(NUM_CHANNEL_PAIRS*SAMPLES_PER_SEC);                      
      //printf("%2.6f, ", time);                                                
      strcpy(buffer, "%2.6f, ", time);                                             
      //printf("Sample %07d: [", i/NUM_CHANNEL_PAIRS);                          
      for(j = 0; j < NUM_CHANNEL_PAIRS-1; j++){                                 
        //power = (data[i+j]/RESISTOR_OHMS)*(LINE_VOLTAGE - data[i+j]);         
        power = (data[i+j]/RESISTOR_OHMS)*LINE_VOLTAGE;                         
        //power = (data[i+j]/RESISTOR_OHMS)*data[i+j];                          
        //power = data[i+j];                                                    
        //printf("%2.6f, ", power);                                             
        strcpy(buffer, "%2.6f, ", power);                                          
      }                                                                         
      power = (data[i+j]/RESISTOR_OHMS)*LINE_VOLTAGE;                           
      //printf("%2.6f]\n", power);                                              
      //printf("%2.6f\n", power);                                               
      strcpy(buffer, "%2.6f\n", power);                                            
    }                                                                           
  }                                                                             
  if( DAQmxFailed(error) ){                                                     
    //printf("DAQmx Error: %s\n",errBuff);                                      
    strcpy(buffer, "DAQmx Error: %s\n",errBuff);                                   
    //return 0;                                                                   
  }                                                                             
                                                                                
  //return 0;






//-----> End of Original Main.c code <-----




    errno = 0;
    // Create the temporary file, this function will replace the 'X's
    filedes = mkstemp(nameBuff);

    // Call unlink so that whenever the file is closed or the program exits
    // the temporary file is deleted
    unlink(nameBuff);

    if(filedes<1)
    {
        printf("\n Creation of temp file failed with error [%s]\n",strerror(errno));
        return 1;
    }
    else
    {
        printf("\n Temporary file [%s] created\n", nameBuff);
    }

    errno = 0;
    // Write some data to the temporary file
    if(-1 == write(filedes,buffer,sizeof(buffer)))
    {
        printf("\n write failed with error [%s]\n",strerror(errno));
        return 1;
    }

    printf("\n Data written to temporary file is [%s]\n",buffer);

    // reset the buffer as it will be used in read operation now
    memset(buffer,0,sizeof(buffer));

    errno = 0;
    // rewind the stream pointer to the start of temporary file
    if(-1 == lseek(filedes,0,SEEK_SET))
    {
        printf("\n lseek failed with error [%s]\n",strerror(errno));
        return 1;
    }

    errno=0;
    // read the data from temporary file
    if( (count =read(filedes,buffer,11)) < 11 )
    {
        printf("\n read failed with error [%s]\n",strerror(errno));
        return 1;
    }

    // Show whatever is read
    fprintf(fp, "\n Data read back from temporary file is:\n [%s]\n",buffer);
    return 0;

//-------- End of Writing to Temporary File Example -----

}


