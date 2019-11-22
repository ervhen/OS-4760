


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysexits.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>




int main(int argc, char *argv[]){

	key_t key;
	int shm;

	key = ftok("oss.c", 'q');
	shm = shmget(key, sizeof(int), 0666);
	
	int *str = (int*) shmat(shm,(void*)0,0);
	
	int check = *str;
	int mydur = atoi(argv[0])+str[0]; 
	
	while(check <= mydur){
		check = *str;
	}

	shmdt(str);
	
	printf("A child left: %d\n", getpid());
	printf("The clock is %d\n", str[0]);
	printf("My duration was %d\n", mydur);




	return 0;
}

