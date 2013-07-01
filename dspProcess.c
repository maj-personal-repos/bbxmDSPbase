#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dspProcess.h"
#include "fdacoefs.h"


// implements an fir filter using fir coefficients defined in fdacoefs.h
// in fixed-point
short fir_filter(buffer *xn){
	int j;
	int yn = 0;
	// performs the convolution of xn with B
	for(j=0; j < BL; j++){
		yn += readn(xn,j)*B[j];
	}
	yn = (yn >> 15) & 0xffff; //converts from Q30 to Q15 fixed point
	return (short)yn; // must cast to a 16-bit short for output to ALSA
}

// core dsp block processing
int dspBlockProcess(short *outputBuffer, short *inputBuffer, buffer *xn, int samples){
	
	memcpy((char *)outputBuffer, (char *)inputBuffer, 2*samples); // passthru
	
	//int i;
	//for (i=0; i < samples; i+=2){
		
		//push(xn,inputBuffer[i]); // stores the most recent sample in the circular buffer xn
		
		//outputBuffer[i] = fir_filter(xn); // filters the input and stores it in the left output channel
		
		//outputBuffer[i+1] = 0; // zeros out the right output channel
	//}
	return DSP_PROCESS_SUCCESS;
}
