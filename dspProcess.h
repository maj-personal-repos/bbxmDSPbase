/*
 *   dspProcess.h
 */
 
#include "buffer.h"

#define     DSP_PROCESS_SUCCESS     0
#define     DSP_PROCESS_FAILURE    -1

int dspBlockProcess(short *outputBuffer, short *inputBuffer, buffer * xnL, buffer *xnR, int samples, int * filter_on, double * volume);
