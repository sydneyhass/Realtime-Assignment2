#include <stdio.h>
#include <stdlib.h>

int main(void) {
//	METRONOME
//	=========
//
//	Reference: look at the controller from Lab7
//	=========
//
//	void metronome_thread()		// create thread in main; thread runs this function
//	=======================		// purpose: "drive" metronome
//	                          //			    receives pulse from interval timer; each time the timer expires
//	                          //          receives pulses from io_write (quit and pause <int>)
//
//	  // Phase I - create a named channel to receive pulses
//	  call attach = name_attach( NULL, "metronome", 0 )
//
//	  calculate the seconds-per-beat and nano seconds for the interval timer
//	  create an interval timer to "drive" the metronome
//	  	configure the interval timer to send a pulse to channel at attach when it expires
//
//	  // Phase II - receive pulses from interval timer OR io_write(pause, quit)
//	  for (;;) {
//	    rcvid = MsgReceive(attach->chid);
//
//	    if ( rcvid == 0 )
//	      switch( msg.pulse_code )
//	        case: METRONOME.pulse_code
//	          //display the beat to stdout
//	          //  must handle 3 cases:
//	          //    start-of-measure: |1
//	          //    mid-measure: the symbol, as seen in the column "Pattern for Intervals within Each Beat"
//	          //    end-of-measure: \n
//
//	       case: PAUSE_PULSE:
//	       		// pause the running timer for pause <int> seconds
//	          // AVOID: calling sleep()
//
//	       case: QUIT_PULSE
//	       	  // implement Phase III:
//	          //  delete interval timer
//	          //  call name_detach()
//	          //  call name_close()
//	          //  exit with SUCCESS
//	  }
//
//	--------------------------------------------------------------------------------------------------------------------------
//
//	ResMgr
//	======
//
//	* io_read()       // copy and paste from Lab7
//	===========
//
//	  //TODO: calculations for secs-per-beat, nanoSecs
//	  sprintf(data, "[metronome: %d beats/min, time signature %d/%d, secs-per-beat: %.2f, nanoSecs: %d]\n",
//
//	  nb = strlen(data);
//
//
//	* io_write()
//	============
//
//	  if (buf == "pause")     // similar to alert <int> from Lab7
//	    process pauseValue
//	    validate pauseValue for range check
//	    MsgSendPulse(metronome_coid, priority, PAUSE_PULSE_CODE, pauseValue);
//
//	  if (buf == "quit")
//	    MsgSendPulse(metronome_coid, priority, QUIT_PULSE_CODE, 0;) // like Lab7
//
//	* io_open()
//	===========
//
//	    metronome_coid = name_open( "metronome", 0 );
//	    return iofunc_default_open(...);
//
//	int main( argv, argc )
//	======================
//
//	  verify number of command-line arguments == 4
//	    if argc != 4
//	    then error message and exit with FAILURE
//
//	  process the command-line arguments:
//	    beats-per-minute
//	    time-signature (top)
//	    time-signature (bottom)
//
//	  implement main(), following simple_resmgr2.c and Lab7 as a guide
//	    device path (FQN): /dev/local/metronome
//	    create the metronome thread in-between calling resmgr_attach() and while(1) { ctp = dispatch_block(... }

}
