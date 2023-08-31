
#include <stdlib.h>
#include <stdio.h>
#include <NIDAQmx.h>

//Includes for writing to temp files
#include<unistd.h>
#include<string.h>
#include<errno.h>

#include<omp.h>

//int main(int argc, char** argv){
int main() {  

  /*
  if(argc != 3){
    printf("Invalid input argument count. Given %d, Expected %d\n", argc-1, 2);
    printf("Usage: ./main [SAMPLES_PER_SECOND] [SAMPLING_TIME_IN_SECONDS]\n");
    exit(-1);
  }
  

  int SAMPLES_PER_SEC = atoi(argv[1]);
  int SAMPLING_SECS = atoi(argv[2]);
  int SAMPLES_PER_CHANNEL = SAMPLING_SECS*SAMPLES_PER_SEC;
  */
  #pragma omp parallel
  printf("Hello from process: %d\n", omp_get_thread_num());

  createTask();
  
  createAIVoltageChan();
  
  setSampleClockAndRate();
 
  startTask();

  takeSamples();

  printf("Start of Loop\n");
 
  long int sum = 0; 
  for (int i = 0; i < 100; i++) {
    for(int j = 0; j < 100; j++) {
      //printf("%d ", i);
      sum = sum + j - i;
      if (i % 1000 == 0) {
        //printf("%d, ", i);
      }
    }
 
  }
  printf("\nSum = %d\n", sum); 
  
  finalize();
   
  return 0;
}


