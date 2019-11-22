

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sysexits.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>


//I just thought this was cool (from timer man pages)
#define errExit(msg)	do{perror(msg); exit(EXIT_FAILURE); } while (0)
#define SIG SIGRTMIN

int cleanflg = 0;


void handler(int sig, siginfo_t *si, void *uc){
	printf("Time is up!");
	cleanflg = 1;
	exit(1);
}


int main(int argc, char *argv[]){
	
	pid_t wpid;

	int status = 0;
	int n, c;
	
	int totchild;
	int totflag = 0;
	int astchild;
	int astflag = 0;

	FILE *input;
	FILE *output;

	char *inputfile;
	int inputset = 0;
	char *outputfile;
	int outputset = 0;

	//make a handler for the timer signal (adapted from timer_create() man page)
	struct sigevent sev;
	struct sigaction sa;
	sigset_t mask;

	sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIG, &sa, NULL) == -1)
		errExit("sigaction");

	//set a timer values 
	struct itimerspec stopwatch;
	stopwatch.it_value.tv_sec = 10;
	stopwatch.it_value.tv_nsec = 0;
	timer_t timerid;

	//more signal stuff
	sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIG;
        sev.sigev_value.sival_ptr = &timerid;

	//timer set
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
			errExit("timer_create");
	timer_settime(timerid, 0, &stopwatch, NULL);



	//parse input commands with getopt
	while ((c = getopt (argc, argv, "hn:s:i:o:")) != -1)
    		switch (c){
      			case 'h':
			        printf("oss [-i inputfile] (if no inputfile is specified, input.txt is the default), [-o outputfile] (if no output file is specified, output.txt is the default), [-h] displays legal options .\n");
                                return 1;
			case 'n':
			        totchild = atoi(optarg);
				totflag = 1;
			        break;
			case 's':
			        astchild = atoi(optarg);
				astflag = 1;
				break;
			case 'i':
			        inputfile = optarg;
				inputset = 1; //flag to tell if inputfile was set or not
			        break;
			case 'o':
			        outputfile = optarg;
				outputset = 1; //flag to tell if outputfile was set or not
			        break;
			case '?':
			        if (optopt == 'c')
			        	fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			        else if (isprint (optopt))
			        	fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			        else
			        	fprintf (stderr,"Unknown option character `\\x%x'.\n", optopt);
        				return 1;
			default:
        			abort ();
      		}
	
	//set inputfile and output to default if flags arnt set, and check other flags.
	if (inputset == 0){
		inputfile = "input.txt";
	}
	if (outputset == 0){
		outputfile = "output.txt";
	}
	if (totflag == 0){
		totchild = 4;
	}
	if (astflag == 0){
		astchild = 2;
	}
	
	//from IBM, stuff for shm
	key_t key;
	int shm;

	//create and attatch shared mem to parent
	key = ftok("oss.c", 'q');
	shm = shmget(key, sizeof(int), 0666|IPC_CREAT);
	int *addr = (int*)shmat(shm,(void*)0,0);



	//open input file.
	int increment;
	input = fopen(inputfile,"r");
	fscanf(input, "%d", &increment);


	//open output for writing.
	output = fopen(outputfile, "w");


	int clock = 0;	
	pid_t pids[astchild];
	int childcount = 0;
	int buff, begin, by, duration, count;
	int numch = 0; //counts number of total children
	int childflag = 0;
	int oflag = 0;
	pid_t check;	
	int hold = 0;


	while (1){
		if (cleanflg == 1){
			while ((wpid = wait(&status)) >0);
			return 0;
		}
		
		clock = clock + increment; //increment the clock
		*addr = clock;
		
		while(childcount < astchild){

			if (childcount < astchild){
				fscanf(input, "%d", &begin);
				fscanf(input, "%d", &by);
				fscanf(input, "%d", &duration);
			}

			if ((begin <= clock) && (by >= clock)){
				numch++;
				childcount++;
				pid_t pid = fork();
				if (pid == 0) {//the child
					char *cargs[2];
					char tebuff[50];
					snprintf(tebuff, 50, "%d", duration);

					cargs[0] = tebuff;
					cargs[1] = NULL;
					printf("in here");	
					execvp("./user", cargs);
					
				}
				else if (pid > 0){ //the parent
					pids[childcount] == pid;
					fprintf(output,"A child was launched at %d with pid %d, the duration passed to it was %d \n", clock, pid, duration);

				}
				else if (pid < 0){ //error
					errExit("forking");
				}		
			}
	

		}
		pid_t endid = waitpid(-1, &status, WNOHANG);
		if (endid != 0){
			childcount = childcount -1;
		}
		if ((numch == totchild) && (childcount == 0)){
			break;
		}					
	}
	
	fclose(output);
	fclose(input);

	shmdt(addr);
	shmctl(shm, IPC_RMID, NULL);



	return 0;
}


