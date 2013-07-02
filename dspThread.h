/*
 *   dspthread.h
 */

// Success and failure definitions for the thread
#define     DSP_THREAD_SUCCESS     ( ( void * ) 0 )
#define     DSP_THREAD_FAILURE     ( ( void * ) - 1 )

// DSP thread environment definition (i.e. what the dsp thread needs to 
// continue to operate)
typedef  struct  dsp_thread_env
{
    int quit;                // Thread will run as long as quit = 0
    int filter_on;			 //
    double volume;
    int done;				 // To capture when the DSP is done.
} dsp_thread_env;

// dspthread function prototypes
void * dsp_thread_start( void * envByRef );
void * ioProcessing(void *envByRef, void *apPtr);
