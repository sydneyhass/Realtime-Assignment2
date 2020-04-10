#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>

int metronome_coid;

typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

#define METRONOME_PULSE 0

#define READ_PULSE 	(METRONOME_PULSE + 1)
#define PAUSE_PULSE	(METRONOME_PULSE + 2)
#define QUIT_PULSE	(METRONOME_PULSE + 3)

char data[255];
int bpm;

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
	//	  configure the interval timer to send a pulse to channel at attach when it expires

	if((nat = name_attach( NULL, "metronome", 0)) == NULL) {
		fprintf(stderr, "Name attach error\n");
		exit(EXIT_FAILURE);
	}

	// Phase II - receive pulses from interval timer OR io_write(pause, quit)
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

	while(1) {
		if((rcvid = MsgReceivePulse(nat->chid, &msg, sizeof(msg), NULL)) == -1) {
			fprintf(stderr, "Message receive pulse error\n");
			exit(EXIT_FAILURE);
		}
		if(rcvid == 0) {
			switch(msg.pulse.code) {
			case METRONOME_PULSE:
				printf();
				break;
			case READ_PULSE:
				break;
			case PAUSE_PULSE:
				break;
			case QUIT_PULSE:
				//  delete interval timer
				if(name_detach(nat, 0) == -1) {
					fprintf(stderr, "Name detach error\n");
					exit(EXIT_FAILURE);
				}
				name_close(metronome_coid);
				return exit(EXIT_SUCCESS);
			default:
				fprintf(stderr, "");
				break;

			}
		}
	}
}

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
	int nb;
	if(data == NULL) return 0;

	//	sprintf(data, "[metronome: %d beats/min, time signature: %d%d, secs-per-beat: %.2f, nanoSecs: %d]\n",
	//			);

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

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
	int nb = 0;

	if( msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg) ))
	{
		char *buf;
		char *pause_msg;
		buf = (char *)(msg+1);
		int i, pauseValue = 0;
		int priority = 0;

		if(strstr(buf, "pause") != NULL) {
			for(i = 0; i < 2; i++) {
				pause_msg = strsep(&buf, " ");
			}
			pauseValue = atoi(pause_msg);
			if(pauseValue >= 1 && pauseValue <= 9) {
				if(MsgSendPulse(metronome_coid, priority, PAUSE_PULSE, pauseValue) == -1) {
					fprintf(stderr, "Error during message send pulse!\n");
					exit(EXIT_FAILURE);
				}
			}
		} else if(strcmp(buf, "quit")) {
			if(MsgSendPulse(metronome_coid, priority, QUIT_PULSE, pauseValue) == -1) {
				fprintf(stderr, "Error during message send pulse!\n");
				exit(EXIT_FAILURE);
			}
		}

		nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES (ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS (0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
	if ((metronome_coid = name_open("metronome", 0)) == -1) {
		perror("name_open failed.\n");
		return EXIT_FAILURE;
	}
	return (iofunc_open_default (ctp, msg, handle, extra));
}

//	  process the command-line arguments:
//	    beats-per-minute
//	    time-signature (top)
//	    time-signature (bottom)
int main(int argc, char* argv[]) {
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t   *ctp;
	int length = sizeof(t) / sizeof(t[0]);

	if(argc != 4) {
		fprintf(stderr, "Error! Wrong number of arguments. Received %d, expected 4.\n", argc);
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < length; ++i){
		if(t[i].time_sig_top == atoi(argv[2]) && t[i].time_sig_bottom == atoi(argv[3])) row = i;
	}

	bpm = atoi(argv[1]);

	int id;
	printf(sizeof(t[0]) + "\n" + sizeof(t[4]));

	if ((dpp = dispatch_create ()) == NULL) {
		fprintf (stderr,
				"%s:  Unable to allocate dispatch context.\n", argv [0]);
		return (EXIT_FAILURE);
	}

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	if ((id = resmgr_attach (dpp, NULL, "/dev/local/metronome",
			_FTYPE_ANY, 0, &connect_funcs, &io_funcs,
			&ioattr)) == -1) {
		fprintf (stderr,
				"%s:  Unable to attach name.\n", argv [0]);
		return (EXIT_FAILURE);
	}

	metronome_thread();

	ctp = dispatch_context_alloc(dpp);
	while(1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}
	return EXIT_SUCCESS;
}
