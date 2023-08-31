SHELL=/bin/sh
BENCHMARK=mg
BENCHMARKU=MG
VEC=

include ../config/make.def

include ../sys/make.common

OBJS = mg.o mg_gpu.o \
       ${COMMON}/c_print_results.o  \
       ${COMMON}/${RAND}.o \
       ${COMMON}/c_timers.o \
       ${COMMON}/wtime.o ${COMMON}/cuda_util.o

${PROGRAM}: config ${OBJS}
	${CLINK} ${CLINKFLAGS} -o ${PROGRAM} ${OBJS} ${C_LIB}

.c.o:
	${CCOMPILE} $< 

mg.o: mg.cu globals.h npbparams.h
	${NVCCCOMPILE} $<

mg_gpu.o: mg_gpu.cu globals.h npbparams.h
	${NVCCCOMPILE} $<

clean:
	- rm -f *.o *~
	- rm -f npbparams.h core
	- if [ -d rii_files ]; then rm -r rii_files; fi
