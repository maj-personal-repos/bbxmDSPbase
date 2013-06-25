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
    audio_params ap = {0};
    ap.initMask = 0x0;
    // minimum block size is 
    ap.exact_bufsize = BLOCKSIZE/BYTESPERFRAME;
    ap.pcm_capture_handle = NULL;
    ap.pcm_output_handle = NULL;
    ap.blksize = BLOCKSIZE;
    ap.right_gain = RIGHT_GAIN;
    ap.left_gain = LEFT_GAIN;
    ap.sample_rate = SAMPLE_RATE;
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
    
    return status;
}

void *ioProcessing(void *envByRef, void *apPtr){
    
    void * status = DSP_THREAD_SUCCESS;
    audio_params *ap = apPtr;
    dsp_thread_env *envPtr = envByRef;
    int err;
    int errcnt =0;
    int i;
    
    // Define circular buffers
    buffer xn;
    initBuffer(&xn);
    
    memset((*ap).outputBuffer, 0, (*ap).blksize);		// Clear the output buffer

    DBG( "Starting IO Processing...\n" );

    //dspBlockProcess((short *)(*ap).outputBuffer, (short *)(*ap).outputBuffer, &xn, (*ap).blksize/2);
    dspBlockProcess((short *)(*ap).outputBuffer, (short *)(*ap).outputBuffer, (*ap).blksize/2);

    DBG( "Entering dspThread processing loop...\n" );

    while( snd_pcm_readi((*ap).pcm_capture_handle, (*ap).inputBuffer, (*ap).exact_bufsize) < 0 ){
		snd_pcm_prepare((*ap).pcm_capture_handle);
		ERR( "<<<<<<<<<<<<<<< Buffer Prime Overrun >>>>>>>>>>>>>>>\n");
		ERR( "Error reading the data from file descriptor %d\n", (int) (*ap).pcm_capture_handle );
	}

    memset((*ap).outputBuffer, 0, (*ap).blksize);		// Clear the output buffer
    
    for(i=0; i<2; i++) {
	    while ((err = snd_pcm_writei((*ap).pcm_output_handle, (*ap).outputBuffer, (*ap).exact_bufsize)) < 0){
			snd_pcm_prepare((*ap).pcm_output_handle);
			ERR( "<<<Pre Buffer Underrun >>> err=%d, errcnt=%d\n", err, errcnt);
		}
	}
	
    while( !(*envPtr).quit ) {
	
		// Read capture buffer from ALSA input device
		while( snd_pcm_readi((*ap).pcm_capture_handle, (*ap).inputBuffer, (*ap).exact_bufsize) < 0 ){
			snd_pcm_prepare((*ap).pcm_capture_handle);
			ERR( "<<<<<<<<<<<<<<< Buffer Prime Overrun >>>>>>>>>>>>>>>\n");
			ERR( "Error reading the data from file descriptor %d\n", (int) (*ap).pcm_capture_handle );
		}
	
		// Audio process
		//  passing the data as short since we are processing 16-bit audio.
		//	memcpy((*ap).outputBuffer, (*ap).inputBuffer, (*ap).blksize);
		//dspBlockProcess((short *)(*ap).outputBuffer, (short *)(*ap).inputBuffer, &xn, (*ap).blksize/2);
		dspBlockProcess((short *)(*ap).outputBuffer, (short *)(*ap).inputBuffer, (*ap).blksize/2);

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

    DBG( "Exited IO Processing loop\n" );
    return status;
}
