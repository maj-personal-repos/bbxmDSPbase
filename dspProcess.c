#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dspProcess.h"

const int BL = 253;
const short B[253] = {
        0,      0,      0,      0,      0,      0,      0,      1,     -1,
        0,      1,     -1,      1,      1,     -2,      2,      0,     -2,
        3,     -1,     -2,      4,     -3,     -1,      5,     -5,      0,
        5,     -7,      2,      5,     -9,      6,      3,    -11,     10,
        0,    -11,     14,     -5,    -10,     18,    -12,     -6,     21,
      -19,      0,     21,    -27,      9,     19,    -33,     21,     11,
      -37,     33,      0,    -37,     46,    -16,    -32,     56,    -35,
      -19,     62,    -56,      0,     61,    -75,     26,     51,    -91,
       56,     31,    -99,     88,      0,    -97,    119,    -40,    -80,
      143,    -88,    -48,    155,   -138,      0,    151,   -186,     63,
      125,   -223,    137,     76,   -244,    217,      0,   -239,    295,
     -101,   -201,    361,   -223,   -124,    403,   -363,      0,    408,
     -511,    177,    361,   -660,    418,    238,   -800,    746,      0,
     -920,   1222,   -455,  -1013,   2074,  -1527,  -1072,   4955,  -8437,
     9831,  -8437,   4955,  -1072,  -1527,   2074,  -1013,   -455,   1222,
     -920,      0,    746,   -800,    238,    418,   -660,    361,    177,
     -511,    408,      0,   -363,    403,   -124,   -223,    361,   -201,
     -101,    295,   -239,      0,    217,   -244,     76,    137,   -223,
      125,     63,   -186,    151,      0,   -138,    155,    -48,    -88,
      143,    -80,    -40,    119,    -97,      0,     88,    -99,     31,
       56,    -91,     51,     26,    -75,     61,      0,    -56,     62,
      -19,    -35,     56,    -32,    -16,     46,    -37,      0,     33,
      -37,     11,     21,    -33,     19,      9,    -27,     21,      0,
      -19,     21,     -6,    -12,     18,    -10,     -5,     14,    -11,
        0,     10,    -11,      3,      6,     -9,      5,      2,     -7,
        5,      0,     -5,      5,     -1,     -3,      4,     -2,     -1,
        3,     -2,      0,      2,     -2,      1,      1,     -1,      1,
        0,     -1,      1,      0,      0,      0,      0,      0,      0,
        0
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
