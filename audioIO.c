/*
 *   audioIO.c
 */
// Modfied for ALSA input/output 18-May-2013, Miguel Alonso Jr.

// Based on Basic PCM audio (http://www.suse.de/~mana/alsa090_howto.html#sect02)http://www.suse.de/~mana/alsa090_howto.html#sect03

//* Standard Linux headers **
#include    <stdio.h>                           // Always include stdio.h
#include    <stdlib.h>                          // Always include stdlib.h
#include    <alsa/asoundlib.h>			// ALSA includes

//* Application headers **
#include     "audioIO.h"             // Audio i/o methods and types
#include     "debug.h"                          // Defines debug routines

//*******************************************************************************
//*  audio_io_setup
//*******************************************************************************
//*  Input parameters:                                                         **
//*   audio_params *apPtr--  A pointer to the audio paramters.				   **
//*                                                                            **
//*  Return value:                                                             **
//*      int  --  AUDIO_SUCCESS or AUDIO_FAILURE as per audio_input_output.h   **
//*                                                                            **
//*******************************************************************************

int audio_io_setup(void * apPtr ){
	
	audio_params * ap = apPtr;
	
	//SND_PCM_STREAM_CAPTURE
	//SND_PCM_STREAM_PLAYBACK
	snd_pcm_stream_t stream_capture = SND_PCM_STREAM_CAPTURE;
	snd_pcm_stream_t stream_playback = SND_PCM_STREAM_PLAYBACK;
	
	snd_pcm_hw_params_t *capture_hwparams;
	snd_pcm_hw_params_t *output_hwparams;
	
	// name of sound device
	char *pcm_name;
	pcm_name = strdup(SOUND_DEVICE);
	
	// allocate memory on the heap for hwparams
	snd_pcm_hw_params_alloca(&capture_hwparams);
	snd_pcm_hw_params_alloca(&output_hwparams);
	
	// open for capture
	DBG( "pcm_capture_handle before snd_pcm_open = %d\n", (int) (*ap).pcm_capture_handle);
	if (snd_pcm_open( &(*ap).pcm_capture_handle, pcm_name, stream_capture, 0) < 0) {
		ERR( "Error opening PCM device %s\n", pcm_name);
		return AUDIO_FAILURE;
    }
	DBG( "pcm_capture_handle after snd_pcm_open = %d\n", (int) (*ap).pcm_capture_handle);
	
	// open for playback
	DBG( "pcm_output_handle before snd_pcm_open = %d\n", (int) (*ap).pcm_output_handle);
    if (snd_pcm_open( &(*ap).pcm_output_handle, pcm_name, stream_playback, 0) < 0) {
		ERR( "Error opening PCM device %s\n", pcm_name);
		return AUDIO_FAILURE;
    }
	DBG( "pcm_output_handle after snd_pcm_open = %d\n", (int) (*ap).pcm_output_handle);
	
	//initialize hw_params
	if (snd_pcm_hw_params_any((*ap).pcm_capture_handle, capture_hwparams) < 0 || snd_pcm_hw_params_any((*ap).pcm_output_handle, output_hwparams)) {
		ERR( "Cannot configure this PCM device.\n");
		return AUDIO_FAILURE;
    }
	
	// used to store exact rate
	unsigned int exact_rate;
	// number of periods
	int periods = 4;
	
	// set to interleaved
	if (snd_pcm_hw_params_set_access((*ap).pcm_capture_handle, capture_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		ERR( "Error setting access.\n");
		return AUDIO_FAILURE;
    }
    
    if (snd_pcm_hw_params_set_access((*ap).pcm_output_handle, output_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		ERR( "Error setting access.\n");
		return AUDIO_FAILURE;
    }
    
    // Sample format
    if (snd_pcm_hw_params_set_format((*ap).pcm_capture_handle, capture_hwparams, SND_PCM_FORMAT_S16_LE) < 0) {
      ERR( "Error setting format.\n");
      return AUDIO_FAILURE;
    }
    
    if (snd_pcm_hw_params_set_format((*ap).pcm_output_handle, output_hwparams, SND_PCM_FORMAT_S16_LE) < 0) {
      ERR( "Error setting format.\n");
      return AUDIO_FAILURE;
    }
    
    // set sampling rate
    exact_rate = (unsigned int)(*ap).sample_rate;
    if (snd_pcm_hw_params_set_rate_near((*ap).pcm_capture_handle, capture_hwparams, &exact_rate, 0u) < 0 || snd_pcm_hw_params_set_rate_near((*ap).pcm_output_handle, output_hwparams, &exact_rate, 0u) < 0) {
		ERR( "Error setting rate.\n");
      return AUDIO_FAILURE;
    }
    if ((*ap).sample_rate != exact_rate) {
		DBG( "The rate %d Hz is not supported by your hardware.\nUsing %d Hz instead.\n", (*ap).sample_rate, exact_rate);
    } else {
		DBG( "Using %d Hz sampling rate.\n", (*ap).sample_rate);
    }
    
	// set number of channels
    if (snd_pcm_hw_params_set_channels((*ap).pcm_capture_handle, capture_hwparams, NUM_CHANNELS) < 0 || snd_pcm_hw_params_set_channels((*ap).pcm_output_handle, output_hwparams, NUM_CHANNELS) < 0) {
      ERR( "Error setting channels.\n");
      return AUDIO_FAILURE;
    }

    // set number of periods
    if (snd_pcm_hw_params_set_periods((*ap).pcm_capture_handle, capture_hwparams, periods, 0) < 0 || snd_pcm_hw_params_set_periods((*ap).pcm_output_handle, output_hwparams, periods, 0) < 0) {
		ERR( "Error setting periods.\n");
		return AUDIO_FAILURE;
    }
	
	// set exacat buffer size
	if (snd_pcm_hw_params_set_buffer_size_near((*ap).pcm_capture_handle, capture_hwparams, &(*ap).exact_bufsize) < 0) {
      ERR( "Error setting buffersize.\n");
      return AUDIO_FAILURE;
    }
    DBG( "exact_bufsize = %d\n", (int) (*ap).exact_bufsize);
	
	// set HW params
	if (snd_pcm_hw_params((*ap).pcm_capture_handle, capture_hwparams) < 0 || snd_pcm_hw_params((*ap).pcm_output_handle, output_hwparams) < 0) {
		ERR( "Error setting HW params.\n");
		return AUDIO_FAILURE;
    }
 
	// allocate memory for input and output buffers
	
	if( ( (*ap).inputBuffer = malloc( (*ap).blksize ) ) == NULL || ( (*ap).outputBuffer = malloc( (*ap).blksize ) ) == NULL  )
    {
        ERR( "Failed to allocate memory for input or output block (%d)\n", (*ap).blksize );
        return AUDIO_FAILURE;
    }
    DBG( "Allocated input and output audio buffers of size %d to address %p and address %p\n", (*ap).blksize, (*ap).inputBuffer, (*ap).outputBuffer );
 
    // return status
    DBG( "Opened %s\n", SOUND_DEVICE);
	
	return AUDIO_SUCCESS;
}

int audio_io_cleanup( void * apPtr ){
	
	audio_params * ap = apPtr;
		
	if(snd_pcm_drain((*ap).pcm_capture_handle) != 0 || snd_pcm_drain((*ap).pcm_output_handle) != 0){
        ERR( "Failed close on ALSA audio input and output devices (input file descriptor %d) (output file descriptor %d) \n", (int) (*ap).pcm_capture_handle, (int) (*ap).pcm_output_handle );
        return AUDIO_FAILURE;
    }
    
    free((*ap).inputBuffer);
    free((*ap).outputBuffer);

    DBG( "Closed audio input and output devices (input file descriptor %d) (output file descriptor %d) \n", (int) (*ap).pcm_capture_handle, (int) (*ap).pcm_output_handle );
    DBG( "Freed audio input and output buffers at locations %p and %p\n", (*ap).inputBuffer, (*ap).outputBuffer );
	
	return AUDIO_SUCCESS;
}
