#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
	int cpid, cexit;
	char *progpath;

	printf("Diese Loesung wurde erstellt von Michael Gutmair\n");
	if(!(argc==1)){
		progpath = (char *) malloc(sizeof(argv[1]));
		strcpy(progpath,argv[1]);
		argv++;
	}	
	else{
		progpath = argv[1];
	}

	
	//here comes the child
	if((cpid = fork()) == 0 ){
		if(argc==1){
			if(execl("./helloWorld","./helloWorld", NULL)==-1){
				printf("Error -1\n");
				exit(-1);
			}
		}
		else{
			if(execv(progpath,argv)==-1){
				printf("Error -1\n");
				exit(-1);
			}
		}
	}
	

	cpid=wait(&cexit);
	printf("Kindprozess %d beendet mit Code %d\n",cpid, cexit);	
	free(progpath);
	return 0;
	
}
