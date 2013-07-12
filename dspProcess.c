#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dspProcess.h"

const int BL = 513;
const int16_T B[513] = {
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,     -1,
        0,      0,      0,      0,      1,      1,      0,      0,      0,
       -1,     -1,     -1,      0,      0,      1,      1,      1,      1,
        0,     -1,     -1,     -2,     -1,     -1,      0,      1,      2,
        2,      1,      0,     -1,     -2,     -2,     -2,     -1,      1,
        2,      3,      3,      2,      0,     -2,     -3,     -3,     -3,
       -1,      1,      3,      4,      4,      2,      0,     -2,     -4,
       -4,     -4,     -2,      1,      3,      5,      5,      3,      0,
       -2,     -4,     -5,     -4,     -2,      1,      4,      5,      5,
        3,      1,     -2,     -4,     -5,     -4,     -2,      1,      3,
        5,      4,      3,      1,     -2,     -3,     -4,     -3,     -1,
        0,      2,      3,      2,      1,      0,     -1,     -1,     -1,
        0,      0,      0,     -1,     -2,     -2,     -2,     -1,      2,
        4,      6,      6,      3,     -1,     -6,    -10,    -11,     -8,
       -2,      6,     13,     17,     15,      8,     -2,    -14,    -22,
      -24,    -18,     -5,     11,     26,     33,     30,     17,     -4,
      -25,    -41,    -44,    -33,    -10,     19,     45,     58,     53,
       29,     -6,    -42,    -68,    -74,    -55,    -17,     31,     72,
       93,     86,     49,     -8,    -66,   -108,   -117,    -89,    -28,
       46,    112,    146,    134,     78,     -9,   -101,   -166,   -182,
     -140,    -47,     69,    172,    227,    212,    125,    -11,   -155,
     -261,   -290,   -225,    -80,    107,    276,    370,    351,    212,
      -12,   -258,   -446,   -507,   -405,   -152,    188,    513,    714,
      702,    447,    -13,   -565,  -1044,  -1274,  -1107,   -474,    598,
     1974,   3438,   4735,   5627,   5944,   5627,   4735,   3438,   1974,
      598,   -474,  -1107,  -1274,  -1044,   -565,    -13,    447,    702,
      714,    513,    188,   -152,   -405,   -507,   -446,   -258,    -12,
      212,    351,    370,    276,    107,    -80,   -225,   -290,   -261,
     -155,    -11,    125,    212,    227,    172,     69,    -47,   -140,
     -182,   -166,   -101,     -9,     78,    134,    146,    112,     46,
      -28,    -89,   -117,   -108,    -66,     -8,     49,     86,     93,
       72,     31,    -17,    -55,    -74,    -68,    -42,     -6,     29,
       53,     58,     45,     19,    -10,    -33,    -44,    -41,    -25,
       -4,     17,     30,     33,     26,     11,     -5,    -18,    -24,
      -22,    -14,     -2,      8,     15,     17,     13,      6,     -2,
       -8,    -11,    -10,     -6,     -1,      3,      6,      6,      4,
        2,     -1,     -2,     -2,     -2,     -1,      0,      0,      0,
       -1,     -1,     -1,      0,      1,      2,      3,      2,      0,
       -1,     -3,     -4,     -3,     -2,      1,      3,      4,      5,
        3,      1,     -2,     -4,     -5,     -4,     -2,      1,      3,
        5,      5,      4,      1,     -2,     -4,     -5,     -4,     -2,
        0,      3,      5,      5,      3,      1,     -2,     -4,     -4,
       -4,     -2,      0,      2,      4,      4,      3,      1,     -1,
       -3,     -3,     -3,     -2,      0,      2,      3,      3,      2,
        1,     -1,     -2,     -2,     -2,     -1,      0,      1,      2,
        2,      1,      0,     -1,     -1,     -2,     -1,     -1,      0,
        1,      1,      1,      1,      0,      0,     -1,     -1,     -1,
        0,      0,      0,      1,      1,      0,      0,      0,      0,
       -1,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0
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
