//Ervin Hennrich
//cs4760 OS
//Assignment 1
//Reads numbers from input file and prints them backwords to output file using forks()
//Uses getopt for options



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

long int fi_procer(FILE *, long int, int *, int, FILE * );

int main (int argc, char *argv[]) {
        pid_t childpid = 0;
        int i, c, n;

	//numbers to read
	int numsread = 0;
	int *numspoint = &numsread;
      
	//for perror string	
	char *thingy = argv[0];
	char *othingy = ":Error";
	char *fthingy = malloc(strlen(thingy) + strlen(othingy) + 1);
	strcpy(fthingy, thingy);
	strcat(fthingy, othingy);
	
	//file stuff
	FILE *fptr; //input file pointer
        char *inputfile = "input.dat";
        char *outputfile = "output.dat";
	int line;
	long int position;
	int starnum;

	//pipe
	int pipefd[2];

        while ((c = getopt (argc, argv, "hi:o:")) != -1)
                switch (c){
                        case 'h':
                                printf("logparse [-i inputfile] (if no inputfile is specified, input.dat is the default), [-o outputfile] (if no output file is specified, output.dat is the default), [-h] displays legal options .\n");
                                return 1;
                        case 'i':
                                inputfile = optarg;
                                break;
                        case 'o':
                                outputfile = optarg;
                                break;
                        case '?':
                                if (optopt = 'c')
                                        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                                else
                                        fprintf(stderr, "Uknown option character '\\x%x'.\n",optopt);
                                return 1;
			default:
				abort();
		}
	fptr = fopen(inputfile, "r");
        if (fptr == NULL){
		perror((fthingy));
                exit(1);
	}
	else if (fptr){
		fscanf(fptr, "%d", &line);
		starnum = line;
		position = ftell(fptr);
	}

	FILE *outer = fopen(outputfile, "w");
	if (outer == NULL){
                perror((fthingy));
                exit(1);
	}
	
	
	long int pidarr[starnum]; //array to hold pids of children

        for (i = 0; i < starnum; i++){
		//the pipe allows the the children to communicate with the parent. The child sends the file position it ends to the parent, the next child inherits this value.
		pipe(pipefd);
                childpid = fork();
		if (childpid > 0) { //the parent
			close(pipefd[1]); //close writing
			pidarr[i] = childpid;
			wait(NULL);
			while (read(pipefd[0], &position, sizeof(long int)) == 1); //get position from child
			close(pipefd[1]);
			//prints child pids and own pid to file
			if (i == (starnum - 1)){
				fprintf(outer, "All children were: ");
				for (i=0; i<starnum; i++)
					fprintf(outer,"%ld ", pidarr[i]);
				fprintf(outer, "\nThe parent pid is: %d", getpid());
			}
		}
		else if (childpid == 0){ //child
			close(pipefd[0]); //close listening
			if (numsread == 0){
				position = fi_procer(fptr, position, numspoint, numsread, outer);
			}
			fprintf(outer, "%d: ", getpid());
			position = fi_procer(fptr, position, numspoint, numsread, outer);
			write(pipefd[1], &position, sizeof(position)); 
			wait(NULL);
			close(pipefd[1]);
			break;
		}
		else
			perror((fthingy));
	}

	fclose(fptr);
	fclose(outer);
	return 0;
}


//processes the file, returns the postion from where it stopped reading
long int fi_procer(FILE *fptr, long int position, int *numspoint, int numsread, FILE *outer){
	int line;
	int i;
	int counter = 0;

	fseek(fptr, position+1, SEEK_SET);
	//gets number of integers to read in
	if (numsread == 0){
		fscanf(fptr, "%d", &line);
		*numspoint = line;
		position = ftell(fptr);
	}
	else {
		int arr[numsread];
		//counts the number of integers read untill it equals the number it was supposed to read (numread)
		while (fscanf(fptr, "%d", &line) != EOF) {
			position = ftell(fptr);
			arr[counter] = line;
			counter++;
			//writes them to output file 
			if (counter == numsread){
				for(i=numsread-1;i >= 0;i--){
					fprintf(outer, "%d ", arr[i]);
				}
				fprintf(outer, "\n");
				break;
			}
		}
	}
	return position;
}

