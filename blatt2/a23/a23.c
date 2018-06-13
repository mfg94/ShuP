#include <stdio.h>
#include <signal.h>

int count =0;
void sighandler(int signal);

int main(int argc, char** argv){

	printf("Diese Loesung wurde erstellt von Michael Gutmair\n");

	
	printf("Programm a23 gestartet...\n");
	printf("Press SIGINT or SIGTERM, after 3rd time program will stop\n");
	

	signal(SIGTERM,sighandler);
	signal(SIGINT,sighandler);
	while(count<3);

	return 5;
}

void sighandler(int signal){
	if(signal==2){
		printf("SIGINT registriert\n");
		count++;
	}
	else if(signal==15){
		printf("SIGTERM registriert\n");
		count++;
	}

}
