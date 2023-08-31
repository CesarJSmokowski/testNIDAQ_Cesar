#!/bin/bash

SAMPLES_PER_SEC=1000
SAMPLING_TIME_IN_SECS=9
START_BENCH_DELAY_SECS=1
OUTPUT_FILE=testout1.csv

echo "Starting Demo! Writing to file: ${OUTPUT_FILE}"

# Start taking samples
./main $SAMPLES_PER_SEC $SAMPLING_TIME_IN_SECS > ${OUTPUT_FILE} &
#./main $SAMPLES_PER_SEC $SAMPLING_TIME_IN_SECS "./Main_OutputV1.txt" > ${OUTPUT_FILE} &
waitpid=$!

# Wait some seconds and run the benchmark
sleep $START_BENCH_DELAY_SECS
./NPB/SNU_NPB_2019/NPB3.3-OMP-C/bin/ft.B.x

#sleep $START_BENCH_DELAY_SECS
#./NPB/SNU_NPB_2019/NPB3.3-OMP-C/bin/ft.B.x
#
#sleep $START_BENCH_DELAY_SECS
#./NPB/SNU_NPB_2019/NPB3.3-OMP-C/bin/ft.B.x

echo "FT Benchmark Complete!"
echo "Waiting for sampling to finish..."

# Wait for sampling to finish
wait $waitpid

echo "Generating image..."

python3 ./plotdata.py

xdg-open allLines.jpg
xdg-open meanLines.jpg

echo "Demo complete!"
