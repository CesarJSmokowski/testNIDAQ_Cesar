#include <stdlib.h>
#include <stdio.h>
#include <NIDAQmx.h>

//Includes for writing to temp files
#include<unistd.h>
#include<string.h>
#include<errno.h>

#include<omp.h>

int main() {  
  #pragma omp parallel
  printf("Hello from process: %d\n", omp_get_thread_num());
  createTask();
  
  createAIVoltageChan();
  
  setSampleClockAndRate();
 
  startTask();

  takeSamples();

  printf("\n");
 
  long int sum = 0; 
  for (int i = 0; i < 1000000; i++) {
    printf("%d ", i);
    sum = sum + i;
  }
  printf("\nSum == %d\n", sum); 
  
  finalize();
   
  return 0;
}


