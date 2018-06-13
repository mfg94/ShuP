#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char** argv){
	
	char cwd[PATH_MAX];
	char input[PATH_MAX];
	char progpath[PATH_MAX];
	char **argv_sub;	
	int exitc;
	char *path;
	char **path_dirs;
	char *ptr;
	int count_dirs;

	
	printf("Diese Loesung wurde erstellt von Michael Gutmair\n");
	printf("Starting Midi-Shell ...\n\n");

while(1){

	//create local path variable and split path directories
	path = (char*) malloc(strlen(getenv("PATH"))+1);
	strcpy(path, getenv("PATH"));
	ptr = path;
	count_dirs=1;
	
	while(*ptr!='\0'){
		if(*ptr==':'){
			count_dirs++;
			*ptr='\0';
		}
	ptr++;
	}	
	path_dirs = (char**) malloc(count_dirs * sizeof(char*));

	path_dirs[0]=path;
	ptr = path;
	for(int i=1; i<count_dirs; i++){
		while(*ptr!='\0'){
			ptr++;
		}
		path_dirs[i]=++ptr;
	}	


	//read cmd
	getcwd(cwd,PATH_MAX);
	printf("Midi-Shell:%s$ ",cwd);
	fgets(input,PATH_MAX,stdin);
	
	if(strcmp(input,"schluss")==0)
		return 0;
	
	if(input[0]=='\n')
		continue;
	
	//count args
	int argc_sub=0;
	char *ptr=input;
	while(*ptr){
		if(*ptr==' '||*ptr=='\n'){
			if(*ptr==' ' && (*(ptr+1)=='\n' || *(ptr+1)==' '))
				*(ptr+1)='\0';
			*ptr='\0';
			argc_sub++;
		}
	ptr++;
	}

	//create string pointer array for args
	argv_sub = (char**) malloc((argc_sub+1) * sizeof(char*));
	argv_sub[0]=input;
	ptr=input;
	for(int i=1; i<argc_sub; i++){
		while(*ptr){
			ptr++;
		}
		argv_sub[i]=++ptr;
	}
	argv_sub[argc_sub]=NULL;

	//cd implementation
	if(strcmp(argv_sub[0], "cd")==0){
		if(chdir(argv_sub[1])==-1)
			printf("Can not find directory!\n");
		free(argv_sub);
		continue;
	}

	//call program
	if(fork()==0){
		//check if we have to lookup in PATH or if its absolute path
		if(strchr(argv_sub[0],'/') || strstr(argv_sub[0],"./") || strstr(argv_sub[0],"../")){//absolute path
			if(execv(argv_sub[0],argv_sub)==-1){
				printf("Fehler im Programmaufruf!\n");
				exit(-1);
			}
			exit(0);
		}
		else{//lookup in PATH variable
			for(int i=0; i<count_dirs; i++){
				strcpy(progpath,path_dirs[i]);
				strcat(progpath,"/");
				strcat(progpath,argv_sub[0]);
				if( access(progpath, F_OK ) != -1 ) {
					// file exists
					if(execv(progpath,argv_sub)==-1){
						printf("Exec fehlgeschlagen mit %s\n",progpath);
						exit(-1);
					}
					else
						exit(0);
				}
			}
			printf("Program not found\n");
			exit(-1);
		}
	}
	
	wait(&exitc);
	free(argv_sub);
	free(path);
	free(path_dirs);
}//while()
}

