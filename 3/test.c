
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



//adapted from stackoverflow and programmingsimplified (without pointers)
char *strrev(char *str)
{
    if (!str || ! *str)
        return str;

    int length = strlen(str) - 1, j = 0;

    char ch;
    while (length > j)
    {
        ch = str[length];
        str[length] = str[j];
        str[j] = ch;
        length--;
        j++;
    }
    return str;
}





int main(int argc, char *argv[]){

	
	time_t rawtime;
	struct tm *timething;

	//the string this child is in charge of
        int linenumber = atoi(argv[1]);
	
	//for random # generation
	time_t gener;
	srand((unsigned) time(&gener));

	struct timespec pillow;
	pillow.tv_sec = 2;

	//semaphore stuff
	
	sem_t *sem1 = sem_open("/seeno", O_RDWR);
	if (sem1 == SEM_FAILED){
		perror("sem1 failed in child");
		exit(-1);
	}
	sem_t *sem2 = sem_open("/hearno", O_RDWR);
	if (sem2 == SEM_FAILED){
		perror("sem2 failed in child");
		exit(-1);
	}

	//from IBM, stuff from shm
	key_t key;
        int shm;
	int sharmemsize = atoi(argv[2]);
        
	//create and attatch shared mem to parent
        key = ftok("thing.c", 'v');
        shm = shmget(key, sharmemsize , IPC_EXCL|0444);
        char *addr = (char*)shmat(shm,(void*)0, SHM_RDONLY);


	int i =0;
	int j = 0;
	int count = 0;
	char memreader[256] = { 0 };

	while (addr[i] != '\0'){
		if (count == linenumber){
			while (addr[i] != ','){
				memreader[j] = addr[i];
				i++;
				j++;
			}
		}
		if(addr[i] == ','){
			count++;
		}
		i++;
	}
	

	int u = 0;
	char hold[256];
	strcpy(hold, memreader); 

	if (strcmp(memreader, strrev(hold)) == 0){ //strcmp appears to be case sensitive
		for(u = 0; u < 5; u++){ 
			sem_wait(sem1);
			time(&rawtime);
			timething = localtime(&rawtime);
			fprintf(stderr, "\n %d going into palin.out with %s at %s", getpid(), memreader, asctime(timething));
			nanosleep(&pillow, NULL);
			//file stuff
			FILE *output;

			output = fopen("palin.out", "a");

			fprintf(output, "%d\t%d\t%s\n", getpid(),linenumber,memreader);
			nanosleep(&pillow, NULL);
			time(&rawtime);
			timething = localtime(&rawtime);
			fprintf(stderr, "\n %d leaving palin.out with %s at %s", getpid(), memreader, asctime(timething));
			fclose(output);
			sem_post(sem1);
		}
	}
	else{ 
		for(u = 0; u < 5; u++){ 
			sem_wait(sem2);
			time(&rawtime);
			timething = localtime(&rawtime);
			fprintf(stderr, "\n %d going into nopalin.out with %s at %s", getpid(), memreader, asctime(timething));
			nanosleep(&pillow, NULL);
			FILE *nopalout;
			nopalout = fopen("nopalin.out","a");
			fprintf(nopalout, "%d\t%d\t%s\n", getpid(),linenumber,memreader);
			nanosleep(&pillow, NULL);
			time(&rawtime);
			timething = localtime(&rawtime);
			fprintf(stderr, "\n %d leaving nopalin.out with %s at %s", getpid(), memreader, asctime(timething));
			fclose(nopalout);
			sem_post(sem2);
		}
	}
	
	shmdt(addr);

	return 0;
}
