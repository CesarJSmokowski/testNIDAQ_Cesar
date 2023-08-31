                                                                                
#include <stdlib.h>                                                             
#include <stdio.h>                                                              
#include <NIDAQmx.h>                                                            
                                                                                
//Includes for writing to temp files                                            
#include<unistd.h>                                                              
#include<string.h>                                                              
#include<errno.h>                                                               
                                                                                
int main(int argc, char** argv){                                                
                                                                                
  if(argc != 3){                                                                
    printf("Invalid input argument count. Given %d, Expected %d\n", argc-1, 2); 
    printf("Usage: ./main [SAMPLES_PER_SECOND] [SAMPLING_TIME_IN_SECONDS]\n");  
    exit(-1);                                                                   
  }                                                                             
                                                                                
  int SAMPLES_PER_SEC = atoi(argv[1]);                                          
  int SAMPLING_SECS = atoi(argv[2]);                                            
  int SAMPLES_PER_CHANNEL = SAMPLING_SECS*SAMPLES_PER_SEC;                      
                                                                                
  createTask();                                                                 
                                                                                
  createAIVoltageChan();                                                        
                                                                                
  setSampleClockAndRate();                                                      
                                                                                
  startTask();                                                                  
                                                                                
  takeSamples();                                                                
                                                                                
  finalize();                                                                   
                                                                                
  return 0;                                                                     
} 
