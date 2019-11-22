#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sysexits.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>



int TIMEFLAG = 0;


void handler(int sir, siginfo_t *si, void *uc){
	TIMEFLAG = 1;
}

int main(){

        int linenumber = 0;

	//signal stuff for timer	
	struct sigevent sev;
        struct sigaction sa;
        sigset_t mask;

        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handler;
        sigemptyset(&sa.sa_mask);//make sure no signals are masked
        if (sigaction(SIGINT, &sa, NULL) == -1)			//want timer and ctrl c to be handled the same
		perror("sigaction");

	//set a timer values
        struct itimerspec stopwatch;
        stopwatch.it_value.tv_sec = 100;
        stopwatch.it_value.tv_nsec = 0;
        stopwatch.it_interval.tv_sec = stopwatch.it_value.tv_sec;
        stopwatch.it_interval.tv_nsec = stopwatch.it_value.tv_nsec;
	timer_t timerid;
        
	//more signal stuff
        sev.sigev_notify = SIGEV_SIGNAL;
//        sev.sigev_signo = SIGRTMIN;
        sev.sigev_signo = SIGINT;	
        sev.sigev_value.sival_ptr = &timerid;
        
	//timer set
        if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
        	perror("timer create");
        if (timer_settime(timerid, 0, &stopwatch, NULL) == -1)
		perror("setting timer");
	
	
	//semaphore stuff
	
	sem_t *sem1 = sem_open("/seeno", O_CREAT, 0666, 1);
	if (sem1 == SEM_FAILED){
		perror("sem1 failed in master");
		exit(-1);
	}
	sem_t *sem2 = sem_open("/hearno", O_CREAT, 0666, 1);
	if (sem2 == SEM_FAILED){
		perror("sem2 failed in master");
		exit(-1);
	}



	FILE *input;
	if ((input = fopen("list","r")) == NULL ){
		perror("Opening file");
		exit(-1);
	}
	
	char s[256];
	//get number of lines
	while(fgets(s, 256, input) != NULL){
		linenumber++;
	}
	
	fseek(input, 0, SEEK_SET);


	//get size of file
	struct stat inputsize;
	stat("list", &inputsize);
	int size = inputsize.st_size;

	char palarr[size*2+1];
	memset(palarr, 0, (size*2+1) * sizeof(char));
	while(fgets(s, 256,input) != NULL){
		strcat(palarr, s);
	}

	fclose(input);

	int y = 0;
	for(y; y<size*2+1; y++){
		if (palarr[y] == '\n'){
			palarr[y] = ',';
		}
	}
        

	//from IBM, stuff from shm
	key_t key;
        int shm;
	int sharmemsize = size * 2 + 10;

        //create and attatch shared mem to parent
        key = ftok("thing.c", 'v');
        shm = shmget(key, sharmemsize , 0644 | IPC_CREAT);
        char *addr = shmat(shm,(void*)0,0);


	strncpy(addr, palarr, sharmemsize);

	int p =0;
        pid_t child;
        int nanny;

        char *snow[4];
	char *myarg1 = "./palin";
	char myarg2[50];
	char myarg3[50];


	sprintf(myarg3, "%d", sharmemsize);
	snow[0] = myarg1;
	snow[2] = myarg3;
	snow[3] = NULL;

        int childcount = 0;

	pid_t childpids[15] = { 0 };
	int f;
	int d;
	while(1){
		if((childcount<15) && (p<linenumber)){
			p=p+1;
			childcount=childcount+1;
			if ((child=fork()) == 0){
					sprintf(myarg2, "%d", p-1);
					snow[1] = myarg2;
					execvp(snow[0], snow);
			}
			else{
				for (d = 0; d < 15; d++){
					if (childpids[d] == 0){
						childpids[d] = child;
						break;
					}
				}
				continue;
			}
		}
//		waitpid( (pid_t) (-1), &nanny, WNOHANG);
		for (f = 0; f < 15; f++){
			if (TIMEFLAG == 1){
				int r = 0;
				printf("Time is up, shutting down now");
				for (r=0; r < 15; r++){
					if (childpids[r] == 0)
						continue;
					else{
						kill(childpids[r], SIGTERM);
					}
				}
				break;
			}
			if (childpids[f] == 0)
				continue;
			else{
				waitpid( childpids[f], &nanny, 0);
                                if (WIFEXITED(nanny)){
                                       childcount= childcount-1;
				       childpids[f] = 0;
                                }
			}
		}

		if ((childcount == 0) || (TIMEFLAG == 1)){
			if((p >= linenumber) || (TIMEFLAG == 1)){
                        	break;
			}
                }
	}


	shmdt(addr);

	shmctl(shm, IPC_RMID, NULL);	
	sem_close(sem1);
	sem_close(sem2);
	sem_unlink("/hearno");
	sem_unlink("/seeno");

	return 0;
}
	
