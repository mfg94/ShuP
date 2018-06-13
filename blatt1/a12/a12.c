#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	printf("\nDiese Lösung wurde erstellt von Michael Gutmair!\n");
	
	if(argc == 1 || argc > 2){
		printf("Genau ein Argument (Zahl) eingeben\n");
		return -1;
	}

	int numberOfWords = strtol(argv[1],NULL,10);
	//sscanf(argv[1], "%i",&numberOfWords);
	printf("%i Words to read\n",numberOfWords);

	char **words = (char**) malloc(sizeof(char*) * numberOfWords);
	for(int i=0; i<numberOfWords; i++){
	words[i] = (char*) malloc(sizeof(char[20]));
	}

	for(int i=0; i<numberOfWords;i++){
		printf("%i. Wort: ",i+1);
		fgets(words[i],19,stdin);
	}

	printf("Ausgabe in umgekehrter Reihenfolge\n-----\n");

	for(int i=numberOfWords-1;i>=0;i--){
		printf("%i. Wort: %s",i+1,words[i]);
	}

	return 0;
}
