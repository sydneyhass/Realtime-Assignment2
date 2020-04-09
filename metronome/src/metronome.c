#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>

//TODO: Reference controller from lab 7
// create thread in main; thread runs this function
// purpose: "drive" metronome
// receives pulse from interval timer; each time the timer expires
// receives pulses from io_write (quit and pause <int>)
typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

#define METRONOME_PULSE 0

#define PAUSE_PULSE	(METRONOME_PULSE + 1)
#define QUIT_PULSE	(METRONOME_PULSE + 2)
#define RESTART_PULSE (METRONOME_PULSE + 3)
#define SET_PULSE 	(METRONOME_PULSE + 4)
#define STOP_PULSE	(METRONOME_PULSE + 5)

char data[255];

struct DataTableRow {
	int time_sig_top;
	int time_sig_bottom;
	int interval_per_beat;
	char* interval;
} typedef DataTableRow;

int row;
struct DataTableRow t[] = {
		{2, 4, 4, "|1&2&"},
		{3, 4, 6, "|1&2&3&"},
		{4, 4, 8, "|1&2&3&4&"},
		{5, 4, 10, "|1&2&3&4-5-"},
		{3, 8, 6, "|1-2-3-"},
		{6, 8, 6, "|1&a2&a"},
		{9, 8, 9, "|1&a2&a3&a"},
		{12, 8, 12, "|1&a2&a3&a4&a"}
};

void metronome_thread() {
	name_attach_t *nat;
	my_message_t msg;
	int rcvid;


	// Phase I - create a named channel to receive pulses
	//
	//	  calculate the seconds-per-beat and nano seconds for the interval timer
	//	  create an interval timer to "drive" the metronome
	//	  	configure the interval timer to send a pulse to channel at attach when it expires
	if((nat = name_attach( NULL, "metronome", 0)) == NULL) {
		fprintf(stderr, "Name attach error\n");
		exit(EXIT_FAILURE);
	}

	while(1) {
		if((rcvid = MsgReceivePulse(nat->chid, &msg, sizeof(msg), NULL)) == -1) {
			fprintf(stderr, "Message receive pulse error\n");
			exit(EXIT_FAILURE);
		}
		if(rcvid == 0) {
			switch(msg.pulse.code) {
			case PAUSE_PULSE:
				break;
			case QUIT_PULSE:
				break;
			case RESTART_PULSE:
				break;
			case SET_PULSE:
				break;
			case STOP_PULSE:
				break;

			}
		}
	}
}



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

//TODO: calculations for secs-per-beat, nanoSecs
//	  sprintf(data, "[metronome: %d beats/min, time signature %d/%d, secs-per-beat: %.2f, nanoSecs: %d]\n",
//
//	  nb = strlen(data);
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
	int nb;
	if(data == NULL) return 0;

	nb = strlen(data);

	//test to see if we have already sent the whole message.
	if (ocb->offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, data, nb);

	//update offset into our data used to determine start position for next read.
	ocb->offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return(_RESMGR_NPARTS(1));
}

//	  if (buf == "pause")     // similar to alert <int> from Lab7
//	    process pauseValue
//	    validate pauseValue for range check
//	    MsgSendPulse(metronome_coid, priority, PAUSE_PULSE_CODE, pauseValue);
//
//	  if (buf == "quit")
//	    MsgSendPulse(metronome_coid, priority, QUIT_PULSE_CODE, 0;) // like Lab7
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
	int nb = 0;

	if( msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg) ))
	{
		/* have all the data */
		char *buf;
		char *alert_msg;
		int i, small_integer;
		buf = (char *)(msg+1);

		if(strstr(buf, "alert") != NULL){
			for(i = 0; i < 2; i++){
				alert_msg = strsep(&buf, " ");
			}
			small_integer = atoi(alert_msg);
			if(small_integer >= 1 && small_integer <= 99){
//				MsgSendPulse(server_coid, SchedGet(0,0,NULL), _PULSE_CODE_MINAVAIL, small_integer);
			} else {
				printf("Integer is not between 1 and 99.\n");
			}
		} else {
			strcpy(data, buf);
		}

		nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES (ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS (0));
}

//	    metronome_coid = name_open( "metronome", 0 );
//	    return iofunc_default_open(...);
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
//	if ((server_coid = name_open("mydevice", 0)) == -1) {
//		perror("name_open failed.\n");
//		return EXIT_FAILURE;
//	}
	return (iofunc_open_default (ctp, msg, handle, extra));
}

//	  process the command-line arguments:
//	    beats-per-minute
//	    time-signature (top)
//	    time-signature (bottom)
//
//	  implement main(), following simple_resmgr2.c and Lab7 as a guide
//	    device path (FQN): /dev/local/metronome
//	    create the metronome thread in-between calling resmgr_attach() and while(1) { ctp = dispatch_block(... }

int main(int argc, char* argv[]) {
	DataTableRow row;

	if(argc != 4) {
		fprintf(stderr, "Error! Wrong number of arguments. Received %d, expected 4.\n", argc);
		exit(EXIT_FAILURE);
	}
}
