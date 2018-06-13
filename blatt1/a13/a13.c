#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct ListElement{

	struct ListElement *prev;
	struct ListElement *next;
	char firstName[20];
	char lastName[20];
	
} ListElement;
void add(ListElement **head, char *firstName, char *lastName);


int main(int argc, char *argv[]){

	printf("\nDiese Lösung wurde erstellt von Michael Gutmair!\n");
	
	if(argc == 1 || argc > 2){
		printf("Genau ein Argument (Zahl) eingeben\n");
		return -1;
	}
	
	int numberOfWords;
	//numberOfWords = strtol(argv[1],NULL,10);
	sscanf(argv[1], "%i",&numberOfWords);
	printf("%i Names to read\n",numberOfWords);

	ListElement *head = NULL;	
	char firstNameBuffer[20];
	char lastNameBuffer[20];

	for(int i=0; i<numberOfWords;i++){
		printf("%i. Vorname: ",i+1);
		scanf("%s",firstNameBuffer);
		printf("%i. Nachname: ", i+1);
		scanf("%s",lastNameBuffer);
		add(&head,firstNameBuffer,lastNameBuffer);		
	}

	printf("Ausgabe in umgekehrter Reihenfolge\n-----\n");

	ListElement *aktElement = head;
	
	do{
		printf("%s %s\n",aktElement->firstName,aktElement->lastName);
		aktElement = aktElement->next;

	}while(aktElement != NULL);



	return 0;
}

void add(ListElement **head, char *firstName, char *lastName)
{
	ListElement *newElement = (ListElement*) malloc(sizeof(ListElement));
	strcpy(newElement->firstName,firstName);
	strcpy(newElement->lastName,lastName);
    
    	newElement->next = (*head);
	newElement->prev = NULL;
 
	if((*head) !=  NULL)
	(*head)->prev = newElement;
 
    	(*head) = newElement;
	printf("Element created\n");
}
