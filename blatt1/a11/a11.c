#include <stdio.h>

int main(int argc, char *argv[]){

	printf("\nDiese Lösung wurde erstellt von Michael Gutmair!\n");
	
	if(argc<=1){
		printf("Mind. 1 Argument eingeben\n");
		return -1;
	}

	for(int i=1; i<=argc-1; i++){
		printf("%i: %s\n", i, argv[i]);
	}
	return 0;
}
