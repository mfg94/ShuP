#include <stdio.h>
#include <unistd.h>


void main(int argc, char* argv[]){

	
	int i=1;
	printf("Dieses Programm wurde erstellt von Michael Gutmair\n");

	while(1){
		sleep(5);
		printf("Schleifendurchlauf %d mit Parameter %s\n",i++,argv[1]);
	}
}
