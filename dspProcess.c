#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dspProcess.h"

const int BL = 187;
const short B[187] = {
        6,     14,     27,     45,     67,     90,    108,    118,    116,
       97,     62,     14,    -40,    -93,   -134,   -156,   -155,   -130,
      -88,    -38,      7,     36,     43,     25,    -13,    -59,   -100,
     -125,   -124,    -96,    -48,      7,     54,     79,     73,     37,
      -20,    -82,   -130,   -147,   -128,    -74,      0,     74,    125,
      136,    101,     29,    -63,   -145,   -194,   -190,   -132,    -32,
       82,    176,    220,    196,    107,    -25,   -164,   -265,   -296,
     -239,   -106,     70,    239,    346,    355,    251,     56,   -179,
     -385,   -495,   -463,   -282,     11,    339,    604,    715,    611,
      289,   -190,   -707,  -1109,  -1242,   -989,   -307,    761,   2081,
     3455,   4656,   5474,   5764,   5474,   4656,   3455,   2081,    761,
     -307,   -989,  -1242,  -1109,   -707,   -190,    289,    611,    715,
      604,    339,     11,   -282,   -463,   -495,   -385,   -179,     56,
      251,    355,    346,    239,     70,   -106,   -239,   -296,   -265,
     -164,    -25,    107,    196,    220,    176,     82,    -32,   -132,
     -190,   -194,   -145,    -63,     29,    101,    136,    125,     74,
        0,    -74,   -128,   -147,   -130,    -82,    -20,     37,     73,
       79,     54,      7,    -48,    -96,   -124,   -125,   -100,    -59,
      -13,     25,     43,     36,      7,    -38,    -88,   -130,   -155,
     -156,   -134,    -93,    -40,     14,     62,     97,    116,    118,
      108,     90,     67,     45,     27,     14,      6
};


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
int dspBlockProcess(short *outputBuffer, short *inputBuffer, buffer *xnL, buffer *xnR, int samples, int * filter_on, double * volume){
	int i;
	if(*filter_on == 0) {
		memcpy((char *)outputBuffer, (char *)inputBuffer, 2*samples); // passthru
	}
	else if(*filter_on == 1) {
		for (i=0; i < samples; i+=2){
			push(xnL,inputBuffer[i]); // stores the most recent sample in the circular buffer xn
			push(xnR,inputBuffer[i+1]);
			outputBuffer[i] = (short)(*volume*fir_filter(xnL)); // filters the input and stores it in the left output channel
			outputBuffer[i+1] = (short)(*volume*fir_filter(xnR)); // zeros out the right output channel
		}	
	}
	return DSP_PROCESS_SUCCESS;
}
