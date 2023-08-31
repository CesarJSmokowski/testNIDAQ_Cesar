
NIDAQLIB_DIR="/usr/lib/x86_64-linux-gnu/"
NIDAQLIB_NAME="nidaqmx"

# Already captured by gcc
#INCLUDE_DIR="/usr/include/"

all: printLoop calibrate

printLoop : printLoop.c ../NPB/SNU_NPB_2019/NPB3.3-OMP-C/FT/ftHelper.o
	gcc -o $@ $^ -L ${NIDAQLIB_DIR} -l ${NIDAQLIB_NAME} -fopenmp

ft.o: ft.c global.h npbparams.h 

calibrate : calibrate.c
	gcc -o $@ $^ -L ${NIDAQLIB_DIR} -l ${NIDAQLIB_NAME}

.PHONY: clean
clean:
	rm -f printLoop	
	rm -f calibrate
