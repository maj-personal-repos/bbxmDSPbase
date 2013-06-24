#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dspProcess.h"
#include "buffer.h"

int dspBlockProcess(short *outputBuffer, short *inputBuffer, buffer * xn, int samples){
	
	//memcpy((char *)outputBuffer, (char *)inputBuffer, 2*samples);
	
	int i;
	for (i=0; i < samples; i+=2){
		push(xn,(float)inputBuffer[i]);
		outputBuffer[i] = (short) pop(xn);
	}
	
	return DSP_PROCESS_SUCCESS;
}
