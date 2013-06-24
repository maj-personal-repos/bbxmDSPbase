/*
 * main.c
 * 
 * Copyright 2013 Miguel Alonso Jr <drmiguelalonsojr@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
 
// Standard headers
#include <stdio.h>
#include <stdlib.h>

// This lets us capture the Ctrl-C interrupt
#include <signal.h>

// Application headers
#include "dspThread.h" // DSP thread definitions
#include "debug.h" // DBG and ERR macros

// Define the global dsp thread environment
dsp_thread_env dsp_env = {0};

/* Store previous signal handler and call it */
void (*pSigPrev)(int sig);

// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
    DBG( "\nCtrl-C pressed, cleaning up and exiting..\n" );
    dsp_env.quit = 1;

    if( pSigPrev != NULL )
        (*pSigPrev)( sig );
}

int main(int argc, char **argv)
{
	int status = EXIT_SUCCESS;
	
	void *dspThreadReturn;
	
	// Set the signal callback for Ctrl-C
    pSigPrev = signal(SIGINT, signal_handler);
    
    // Starts the DSP thread
    dspThreadReturn = dsp_thread_start( (void *) &dsp_env );
    
	DBG("DSP Status is: %d .\n", (int) dspThreadReturn);
	
	return status;
}

