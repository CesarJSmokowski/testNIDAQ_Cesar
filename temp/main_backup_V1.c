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
    printf("time, ");                                                           
    for(i = 0; i < NUM_CHANNEL_PAIRS-1; i++){                                   
      printf("line%d, ",i);                                                     
    }                                                                           
    printf("line%d\n",i);                                                       
                                                                                
    // Print the data                                                           
    for(i = 0; i < ARRAY_SIZE_IN_SAMPLES; i+=NUM_CHANNEL_PAIRS){                
      time = (float)i/(NUM_CHANNEL_PAIRS*SAMPLES_PER_SEC);                      
      printf("%2.6f, ", time);                                                  
      //printf("Sample %07d: [", i/NUM_CHANNEL_PAIRS);                          
      for(j = 0; j < NUM_CHANNEL_PAIRS-1; j++){                                 
        //power = (data[i+j]/RESISTOR_OHMS)*(LINE_VOLTAGE - data[i+j]);         
        power = (data[i+j]/RESISTOR_OHMS)*LINE_VOLTAGE;                         
        //power = (data[i+j]/RESISTOR_OHMS)*data[i+j];                          
        //power = data[i+j];                                                    
        printf("%2.6f, ", power);                                               
      }                                                                         
      power = (data[i+j]/RESISTOR_OHMS)*LINE_VOLTAGE;                           
      //printf("%2.6f]\n", power);                                              
      printf("%2.6f\n", power);                                                 
    }                                                                           
  }                                                                             
  if( DAQmxFailed(error) ){                                                     
    printf("DAQmx Error: %s\n",errBuff);                                        
    return 0;                                                                   
  }                                                                             
                                                                                
  return 0;                                                                     
}                                                                               
                                                                                
                                                                                
"main.c" 183L, 5955C                                                                                                                             183,0-1       Bot
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
    printf("time, ");                                                           
    for(i = 0; i < NUM_CHANNEL_PAIRS-1; i++){                                   
      printf("line%d, ",i);                                                     
    }                                                                           
    printf("line%d\n",i);                                                       
                                                                                
    // Print the data                                                           
    for(i = 0; i < ARRAY_SIZE_IN_SAMPLES; i+=NUM_CHANNEL_PAIRS){                
      time = (float)i/(NUM_CHANNEL_PAIRS*SAMPLES_PER_SEC);                      
      printf("%2.6f, ", time);                                                  
      //printf("Sample %07d: [", i/NUM_CHANNEL_PAIRS);                          
      for(j = 0; j < NUM_CHANNEL_PAIRS-1; j++){                                 
        //power = (data[i+j]/RESISTOR_OHMS)*(LINE_VOLTAGE - data[i+j]);         
        power = (data[i+j]/RESISTOR_OHMS)*LINE_VOLTAGE;                         
        //power = (data[i+j]/RESISTOR_OHMS)*data[i+j];                          
        //power = data[i+j];                                                    
        printf("%2.6f, ", power);                                               
      }                                                                         
      power = (data[i+j]/RESISTOR_OHMS)*LINE_VOLTAGE;                           
      //printf("%2.6f]\n", power);                                              
      printf("%2.6f\n", power);                                                 
    }                                                                           
  }                                                                             
  if( DAQmxFailed(error) ){                                                     
    printf("DAQmx Error: %s\n",errBuff);                                        
    return 0;                                                                   
  }                                                                             
                                                                                
  return 0;                                                                     
}                                                                               
                                                                                
                                                                                
                                                                                                                              183,0-1       Bot

