
NIDAQLIB_DIR="/usr/lib/x86_64-linux-gnu/"
NIDAQLIB_NAME="nidaqmx"

# Already captured by gcc
#INCLUDE_DIR="/usr/include/"

all: empty calibrate

empty : empty.c ../NPB/SNU_NPB_2019/NPB3.3-OMP-C/FT/ftHelper.o
	gcc -o $@ $^ -L ${NIDAQLIB_DIR} -l ${NIDAQLIB_NAME}

calibrate : calibrate.c
	gcc -o $@ $^ -L ${NIDAQLIB_DIR} -l ${NIDAQLIB_NAME}

.PHONY: clean
clean:
	rm -f main	
	rm -f calibrate
