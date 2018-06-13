#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char** argv){

	printf("Diese Loesung wurde erstellt von Michael Gutmair\n");
	
	fork();
	printf("Prozess %d mit Vater %d\n",getpid(),getppid());

	while(1);
	return 0;
}
