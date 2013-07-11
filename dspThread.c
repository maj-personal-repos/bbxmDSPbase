/*
 *   dspthread.c
 */

// Standard headers
#include	<stdio.h> // Always include stdio.h
#include	<stdlib.h> // Always include stdlib.h
#include 	<alsa/asoundlib.h> // ALSA includes
#include	<string.h> // For memcpy

// Application headers
#include "debug.h" // DBG and ERR macros
#include "dspThread.h" // DSP thread definitions
#include "audioIO.h" // ALSA Audio Setup
#include "dspProcess.h" // DSP Algorithm definitions
#include "buffer.h" // Circular Buffer

//*******************************************************************************
//*  dsp_thread_start                                                          **
//*******************************************************************************
//*  Input Parameters:                                                         **
//*      void *envByRef    --  a pointer to an dsp_thread_env structure        **
//*                            as defined in dspthread.h                       **
//*                                                                            **
//*          envByRef.quit -- when quit != 0, thread will cleanup and exit     **
//*                                                                            **
//*  Return Value:                                                             **
//*      void *            --  DSP_THREAD_SUCCESS or DSP_THREAD_FAILURE as     **
//*                            defined in dspthread.h                          **
//*******************************************************************************

void *dsp_thread_start( void *envByRef )
{	
	// DSP Thread Parameters
	dsp_thread_env * envPtr = envByRef; // < see above >
    void * status = DSP_THREAD_SUCCESS; // < see above >
    
    // Define audio paramters
    audio_params ap = {0}; // initialize ap
    ap.initMask = 0x0; // set init mask to 0x0
    ap.exact_bufsize = BLOCKSIZE/BYTESPERFRAME;// set buffer size
    ap.pcm_capture_handle = NULL;
    ap.pcm_output_handle = NULL;
    ap.blksize = BLOCKSIZE; // set block size
    ap.right_gain = RIGHT_GAIN; // set left channel gain
    ap.left_gain = LEFT_GAIN; // set right channel gain
    ap.sample_rate = SAMPLE_RATE; // set sampling rate
    ap.inputBuffer = NULL;
    ap.outputBuffer = NULL;
    
    // Setup audio codec and I/O buffers
    if( audio_io_setup( (void *) &ap ) == AUDIO_FAILURE )
    {
        ERR( "Audio_input_setup failed in dsp_thread_start\n\n" );
        status = DSP_THREAD_FAILURE;
    }
    
    // Perform I/O and Processing
	ioProcessing(envByRef,&ap);

    // Cleanup: Free up resources
    if( audio_io_cleanup( (void *) &ap ) == AUDIO_FAILURE )
    {
        ERR( "Audio_io_cleanup failed in dsp_thread_start\n\n" );
        status = DSP_THREAD_FAILURE;
    }
    
    envPtr->done = 1;
    
    // Finish
    return status;
}

void *ioProcessing(void *envByRef, void *apPtr){
    
    // Key variables for DSP
    void * status = DSP_THREAD_SUCCESS;
    audio_params *ap = apPtr; // Gets the audio parameters
    dsp_thread_env *envPtr = envByRef; // Gets the dsp thread environment
    int err; // for capturing errors
    int errcnt =0; // for capturing errors
    int *filter_on = malloc(sizeof(int));
    double * volume = malloc(sizeof(double));
    int i;
    
    buffer *xnL = malloc(sizeof(buffer)); // Define circular buffers and allocate memory left channel xn
    buffer *xnR = malloc(sizeof(buffer)); // Define circular buffers and allocate memory right channel xn
    buffer *ynL = malloc(sizeof(buffer)); // Define circular buffers and allocate memory left channel yn
    buffer *ynR = malloc(sizeof(buffer)); // Define circular buffers and allocate memory right channel yn
    initBuffer(xnL); // Initialize circular buffer
    initBuffer(xnR); // Initialize circular buffer
    initBuffer(ynL); // Initialize circular buffer
    initBuffer(ynR); // Initialize circular buffer
    *filter_on = (*envPtr).filter_on;
    *volume = (*envPtr).volume;
    
    memset((*ap).outputBuffer, 0, (*ap).blksize); // Clear the output buffer

    DBG( "Starting IO Processing...\n" );

	// Process a block just to start the DSP and skip the first frame
    dspBlockProcess((short *)(*ap).outputBuffer, (short *)(*ap).outputBuffer, xnL, xnR, (*ap).blksize/2, filter_on, volume );

    DBG( "Entering dspThread processing loop...\n" );

	// Read capture buffer from ALSA input device (just to get things going) 
    while( snd_pcm_readi((*ap).pcm_capture_handle, (*ap).inputBuffer, (*ap).exact_bufsize) < 0 ){
		snd_pcm_prepare((*ap).pcm_capture_handle);
		ERR( "<<<<<<<<<<<<<<< Buffer Prime Overrun >>>>>>>>>>>>>>>\n");
		ERR( "Error reading the data from file descriptor %d\n", (int) (*ap).pcm_capture_handle );
	}

    memset((*ap).outputBuffer, 0, (*ap).blksize);		// Clear the output buffer
    
    // Write output buffer into ALSA output device (just to get things going)
    for(i=0; i<2; i++) {
	    while ((err = snd_pcm_writei((*ap).pcm_output_handle, (*ap).outputBuffer, (*ap).exact_bufsize)) < 0){
			snd_pcm_prepare((*ap).pcm_output_handle);
			ERR( "<<<Pre Buffer Underrun >>> err=%d, errcnt=%d\n", err, errcnt);
		}
	}
	
	// begin DSP main loop
    while( !(*envPtr).quit ) {
		
		// check dynamic DSP parameters
		*filter_on = (*envPtr).filter_on;
		*volume = (*envPtr).volume;
	
		// Read capture buffer from ALSA input device
		while( snd_pcm_readi((*ap).pcm_capture_handle, (*ap).inputBuffer, (*ap).exact_bufsize) < 0 ){
			snd_pcm_prepare((*ap).pcm_capture_handle);
			ERR( "<<<<<<<<<<<<<<< Buffer Prime Overrun >>>>>>>>>>>>>>>\n");
			ERR( "Error reading the data from file descriptor %d\n", (int) (*ap).pcm_capture_handle );
		}
		
		// Audio process
		//  passing the data as short since we are processing 16-bit audio.
		dspBlockProcess((short *)(*ap).outputBuffer, (short *)(*ap).inputBuffer, xnL, xnR, (*ap).blksize/2, filter_on, volume);

		// Write output buffer into ALSA output device
		errcnt = 0;	
		// The Beagle gets an underrun error the first time it trys to write,
		// so ignore the first error and it appears to work fine.
		while ((err = snd_pcm_writei((*ap).pcm_output_handle, (*ap).outputBuffer, (*ap).exact_bufsize)) < 0) {
			snd_pcm_prepare((*ap).pcm_output_handle);
			ERR( "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>> err=%d, errcnt=%d\n", err, errcnt);
			memset((*ap).outputBuffer, 0, (*ap).blksize);		// Clear the buffer
			snd_pcm_writei((*ap).pcm_output_handle, (*ap).outputBuffer, (*ap).exact_bufsize);
		}
    }
	
	
	// Free up resources
	destroyBuffer(xnL);
	free(xnL);
	destroyBuffer(xnR);
	free(xnR);
	destroyBuffer(ynL);
	free(ynL);
	destroyBuffer(ynR);
	free(ynR);
	free(filter_on);
	free(volume);

    DBG( "Exited IO Processing loop\n" );
    return status;
}
