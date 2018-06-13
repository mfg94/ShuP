#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>


int main(int argc, char** argv){

	printf("Diese Loesung wurde erstellt von Michael Gutmair\n");
	int c1pid, c2pid, c3pid;
	int cexit, endcpid;
	int ppid = getpid();

	if(argc != 5){
		printf("Mit genau vier Übergabeparametern starten\n");
		return 0;
	}

	printf("Argument 1: %s ppid: %d\n", argv[1], getpid());	

	c1pid = fork();
	if(ppid == getpid()){ c2pid=fork(); }
	if(ppid == getpid()){ c3pid=fork(); }
	
	//Father comes here
	if(ppid == getpid()){
		sleep(2);
		
		kill(c1pid,15);
		kill(c2pid,9);
		kill(c3pid,9);

		for(int i=0; i<3; i++){
			endcpid = wait(&cexit);	
			printf("%d ended with exit: %d\n",endcpid,cexit);
		}

		exit(0);	
		}
	
	//child1 goes here
	if(!c1pid){
		printf("Argument 2: %s c1pid: %d\n", argv[2], getpid());
		while(1);
	}
	
	//child2 goes here
	if(!c2pid){
		printf("Argument 3: %s c2pid: %d\n", argv[3], getpid());
		while(1);
	}

	//child3 goes here
	if(!c3pid){
		printf("Argument 4: %s c3pid: %d\n", argv[4], getpid());
		sleep(1);
		exit(2);
	}	

	exit(0);
}
