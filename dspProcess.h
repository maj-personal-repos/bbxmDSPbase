/*
 *   dspProcess.h
 */
 
#include "buffer.h"

#define     DSP_PROCESS_SUCCESS     0
#define     DSP_PROCESS_FAILURE    -1

//int dspBlockProcess(short *outputBuffer, short *inputBuffer, buffer * xn, int samples);
int dspBlockProcess(short *outputBuffer, short *inputBuffer, int samples);
