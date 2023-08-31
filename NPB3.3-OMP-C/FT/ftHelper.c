#include <stdio.h> 
#include "ftHelper.h"

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) finalize(); else

  void createTask() {

    //SAMPLES_PER_SEC = 1000; //atoi(argv[1]);
    //SAMPLING_SECS = 9; //atoi(argv[2]);
    //SAMPLES_PER_CHANNEL = SAMPLING_SECS*SAMPLES_PER_SEC;
    
    //error = 0;
    //taskHandle = 0;
    //float64     data[ARRAY_SIZE_IN_SAMPLES];
    //char        errBuff[2048]={'\0'};

    printf("---> createTask() Called \n");
    printf("DAQ_TASK_NAME = %s\n", DAQ_TASK_NAME);
    printf("taskHandle = %d\n", taskHandle);
    printf("&taskHandle = %d\n", &taskHandle);
    //DAQmxStopTask(taskHandle);
    int32 createTaskStatus = DAQmxCreateTask(DAQ_TASK_NAME, &taskHandle);
    printf("createTaskStatus = %d\n", createTaskStatus);
    DAQmxErrChk (createTaskStatus);
    printf("After createTask() ---> taskHandle = %d\n", taskHandle);
  }

  void createAIVoltageChan() {
    printf("---> createAIVoltageChan (ftHelper.c)\n");
    // Start in differential mode                                                 
    DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle,                              
                                       PHYS_CHANNELS,                           
                                       //"cDAQ1Mod3/ai0:3, cDAQ1Mod3/ai8:11",   
                                       CHANNEL_NAME,                            
                                       DAQmx_Val_Diff,                          
                                       //DAQmx_Val_NRSE,                        
                                       //DAQmx_Val_RSE,                         
                                       MIN_VOLTS, MAX_VOLTS,                    
                                       DAQmx_Val_Volts, NULL));                 
                                                                                
    printf("Voltage Chan created!\n"); 
    printf("createAIVoltageChan() ---> taskHandle = %d\n", taskHandle);

  }

  void setSampleClockAndRate() {
    // Setup the sample clock and the rate at which we collect samples
    printf("---> setSampleClockRate() Called\n");
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, NULL, SAMPLES_PER_SEC,          
                                    DAQmx_Val_Rising,                           
                                    DAQmx_Val_FiniteSamps,                      
                                    SAMPLES_PER_CHANNEL ));
    printf("setSampleClockAndRate() ---> taskHandle = %d\n", taskHandle);
    printf("Sampling rate set!\n");
  }

  void startTask() {
    // DAQmx Start Code
    printf("---> startTask() Called\n");                                                           
    DAQmxStartTask(taskHandle);
    printf("startTask() ---> taskHandle = %d\n", taskHandle);
    printf("Task started!\n");
  }

  void finalize() {
    //Error:
    printf("---> finalize() Called\n");
    printf("finalize() ---> taskHandle = %d", taskHandle);
    // DAQmx Stop and clear task
    if( DAQmxFailed(error) ){
    DAQmxGetExtendedErrorInfo(errBuff,2048);
  }
  if( taskHandle != 0 )  {
    printf("NIDAQ testing complete!\n");

    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);

    //printf("We read [%d] samples for each channel\n", samples_read_per_channel);
    //printf("We took [%d] samples per second\n", SAMPLES_PER_SEC);

    // Print out the data we collected on differences across the paired pins

    float64 power;
    float64 time;
    int i,j;

    // Print the header of the csv
    printf("time, ");
    //fprintf(fp, "time, ");

    for(i = 0; i < NUM_CHANNEL_PAIRS-1; i++){
      printf("line%d, ",i);
      //fprintf(fp, "line%d, ",i);
    }
    printf("line%d\n",i);
    //fprintf(fp, "line%d\n",i);

    // Print the data
    for(i = 0; i < ARRAY_SIZE_IN_SAMPLES; i+=NUM_CHANNEL_PAIRS){
      time = (float)i/(NUM_CHANNEL_PAIRS*SAMPLES_PER_SEC);
      printf("%2.6f, ", time);
      //fprintf(fp, "%2.6f, ", time);
      //printf("Sample %07d: [", i/NUM_CHANNEL_PAIRS);
      for(j = 0; j < NUM_CHANNEL_PAIRS-1; j++){
        //power = (data[i+j]/RESISTOR_OHMS)*(LINE_VOLTAGE - data[i+j]);
        power = (data[i+j]/RESISTOR_OHMS)*LINE_VOLTAGE;
        //power = (data[i+j]/RESISTOR_OHMS)*data[i+j];
        //power = data[i+j];
        printf("%2.6f, ", power);
        //fprintf(fp, "%2.6f, ", power);
      }
      power = (data[i+j]/RESISTOR_OHMS)*LINE_VOLTAGE;
      //printf("%2.6f]\n", power);
      printf("%2.6f\n", power);
      //fprintf(fp, "%2.6f\n", power);
    }
  }
  if( DAQmxFailed(error) ){
    printf("DAQmx Error: %s\n",errBuff);
    //fprintf(fp, "DAQmx Error: %s\n",errBuff);

    //return 0;
  }

  //rewind() function sets the file pointer at the beginning of the sream
  /*
  rewind(fp);

  while (!feof(fp))
    putchar(fgetc(fp)); 
  */
  }


  void takeSamples() {                                                          
    // DAQmx Read Code -- i.e: take samples                               
  printf("---> takeSamples() Called\n");        
  // The samples are written interleaved with the GroupByScanNumber             
  DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, SAMPLES_PER_CHANNEL,               
                                 SAMPLES_WAIT_TIMEOUT_SECS,                     
                                 DAQmx_Val_GroupByScanNumber,                   
                                 //DAQmx_Val_GroupByChannel,                    
                                 data, ARRAY_SIZE_IN_SAMPLES,                   
                                 &samples_read_per_channel,                     
                                 NULL));                                        
    
    printf("takeSamples() ---> taskHandle = %d\n", taskHandle);
    printf("---> Reached End of takeSamples()\n");
    //finalize();                                                                 
  }
