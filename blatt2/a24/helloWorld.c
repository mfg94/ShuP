#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int count =0;
void sighandler(int signal);

int main(int argc, char** argv){
	int cpid, ppid;

	printf("Unterprogramm: Hello World programmiert von Michael Gutmair\n");

	for(int i=0; i<argc; i++){
		printf("Aufrufparameter %d: %s\n",i,argv[i]);
	}

	printf("Kindprozess wartet 3s...\n");
	sleep(3);	
	return 0;
	

}
