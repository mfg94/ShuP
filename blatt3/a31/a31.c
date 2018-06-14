#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>

#define MAIN "<MAIN>"
#define APP "\t\t<APP %d>"
#define SPOO "\t\t\t\t<SPOOL>"
#define PRINT "\t\t\t\t\t\t\t<PRINTER %d>"

typedef struct job{
    int jobId;
	int printTime;
	int pages;
	int content;
} job;

//declare shared memory and semaphore handler
int shmSpoolerQueue, shmSpoolerPosition, shmPrinterQueue;
int semSpoolerFull, semSpoolerEmpty, mutexSpooler, semPrinterFull, semPrinterEmpty;

void spooler();
void app(int printTime, int content, int pages);
void printer(int printerNr);

#define QUEUE_SIZE 5
#define PRINTERS 2

int main(int argc, char** argv){

	pid_t spoolerPid, taskPid, printerPid[PRINTERS];
    char* endptr;
    int* spoolerPosition;
	int numberOfTasks, printTime, content, pages, taskExitStatus, waitTime,  i, retVal;

	printf("Diese Loesung wurde erstellt von Michael Gutmair\n");

	if((argc <=1) || (numberOfTasks = strtol(argv[1], &endptr, 10)) <= 0){
        printf(MAIN"Please provide an integer as first argument!\n");
        return -1;
    }

	//initialize random
	srand(time(NULL));

    //initialize semaphores and shared memory
    shmSpoolerQueue = shmget(IPC_PRIVATE, QUEUE_SIZE * sizeof(job), 0777 | IPC_CREAT);//createing sharedmemory for spoolerqueue
    shmSpoolerPosition = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT); //shared memory for current position in spoolerqueue
   	shmPrinterQueue = shmget(IPC_PRIVATE, PRINTERS * sizeof(job), 0777 | IPC_CREAT); //array for printer queue

    if(shmSpoolerPosition == -1 || shmSpoolerPosition == -1 || shmPrinterQueue == -1){
        printf(MAIN"ERROR Creating shared memory... Exiting...\n");
        exit(-1);
    }

    semSpoolerFull = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //sem spoolerFull;
    semSpoolerEmpty = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //sem spoolerEmpty;
    mutexSpooler = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //mutex spooler;
	semPrinterFull = semget(IPC_PRIVATE, PRINTERS, 0777 | IPC_CREAT); //sem printerFull[2];
	semPrinterEmpty = semget(IPC_PRIVATE, PRINTERS, 0777 | IPC_CREAT); //sem printerEmpty[2];
    if(semSpoolerFull == -1 || semSpoolerEmpty == -1 || mutexSpooler == -1 || semPrinterEmpty == -1 || semPrinterFull == -1){
        printf(MAIN"ERROR Creating semaphores... Exiting...\n");
        exit(-1);
    }

   	if(semctl(semSpoolerFull, 0, SETVAL, 0) == -1){ //sem spoolerFull = 0
       printf(MAIN"ERROR Setting semSpoolerFull... Exiting...\n");
        exit(-1);
    }
   	if(semctl(semSpoolerEmpty, 0, SETVAL, QUEUE_SIZE) == -1){ //sem spoolerEmpty = 5
       printf(MAIN"ERROR Setting semSpoolerEmpty... Exiting...\n");
        exit(-1);
    }
	if(semctl(mutexSpooler, 0, SETVAL, 1) == -1){ //mutex spooler = 1
        printf(MAIN"ERROR Setting mutexSpooler... Exiting...\n");
        exit(-1);
    }
	if(semctl(semPrinterFull, 0, SETVAL, 0) == -1){ //sem printerFull[0] = 0
        printf(MAIN"ERROR Setting semPrinterFull... Exiting...\n");
        exit(-1);
    }
	if(semctl(semPrinterFull, 1, SETVAL, 0) == -1){ //sem printerFull[1] = 0
        printf(MAIN"ERROR Setting semPrinterFull... Exiting...\n");
        exit(-1);
    }
	if(semctl(semPrinterEmpty, 0, SETVAL, 1) == -1){ //sem printerEmpty[0] = 1
        printf(MAIN"ERROR Setting semPrinterEmpty... Exiting...\n");
        exit(-1);
    }
	if(semctl(semPrinterEmpty, 1, SETVAL, 1) == -1){ //sem printerEmpty[1] = 1
        printf(MAIN"ERROR Setting semPrinterEmpty... Exiting...\n");
        exit(-1);
    }

    if((spoolerPosition = shmat(shmSpoolerPosition, NULL, 0)) == (void *)-1){
        printf(MAIN"ERROR Getting spoolerPosition... Exiting...\n");
        exit(-1);
    }
   	*spoolerPosition = 0; //set current position for spoolerqueue initially to 0

	//Start spooler
    if((spoolerPid = fork()) == 0){
        printf(MAIN"Spooler started...\n");
        spooler();
        exit(0);
    }

    //Start Printers
    if((printerPid[0] = fork()) == 0){
        printer(0);
        exit(0);
    }

    if((printerPid[1] = fork()) == 0){
        printer(1);
        exit(0);
    }

	//starts apps
	for(i=0; i<numberOfTasks; i++){

        waitTime = rand() % 5; //wait until start of next job
        sleep(waitTime);

      	printTime = rand() % 5 + 2;
		content = rand();
		pages = printTime * 2;

        if((taskPid = fork()) == 0){
            printf(MAIN"Starting app %d...\n", getpid());
			app(printTime, content, pages);
			exit(0);
        }
	}
    waitpid(taskPid, &taskExitStatus, 0);
	printf(MAIN"All apps created...\n");	

    if((taskPid = fork()) == 0){
            printf(MAIN"Adding empty dummy job to end spooler and printer...\n");
			app(-1, -1, -1);
			exit(0);
    }
    waitpid(taskPid, &taskExitStatus, 0);

    printf(MAIN"Waiting for spooler to end...\n");
	waitpid(spoolerPid, &taskExitStatus, 0);
    printf(MAIN"Spooler ended...\n");
    printf(MAIN"Waiting for printer 0 to end...\n");
	waitpid(printerPid[0], &taskExitStatus, 0);
    printf(MAIN"Printer 0 ended...\n");
    printf(MAIN"Waiting for printer 1 to end...\n");
	waitpid(printerPid[1], &taskExitStatus, 0);
	printf(MAIN"Printer 1 ended...\n");

	shmdt((void*) spoolerPosition);

    //delete shared memory and semaphores
	shmctl(shmSpoolerQueue, IPC_RMID, NULL);
	shmctl(shmSpoolerPosition, IPC_RMID, NULL);
    shmctl(shmPrinterQueue, IPC_RMID, NULL);
	semctl(semSpoolerEmpty, 0, IPC_RMID, NULL);
    semctl(semSpoolerFull, 0, IPC_RMID, NULL);
    semctl(mutexSpooler, 0, IPC_RMID, NULL);
    semctl(mutexSpooler, 1, IPC_RMID, NULL);
    semctl(semPrinterFull, 0, IPC_RMID, NULL);
    semctl(semPrinterEmpty, 0, IPC_RMID, NULL);

	return 0;

}


void app(int printTime, int content, int pages){

    job* queue;
    job newJob;
    int* queuePosition;
    int taskId;
    struct sembuf w, s;

    taskId = getpid();
    
    //wait sembuf
    w.sem_num = 0;
    w.sem_op = -1;
    w.sem_flg = 0;
    
    //signal sembuf
    s.sem_num = 0;
    s.sem_op =  1; 
    s.sem_flg = 0;

    if((queue = shmat(shmSpoolerQueue, NULL, 0)) == (void*) -1){
        printf(APP"ERROR getting shmSpoolerQueue... Exiting...\n", taskId);
        exit(-1);
    }
    if((queuePosition = shmat(shmSpoolerPosition, NULL, 0)) == (void*) -1){
        printf(APP"ERROR getting shmSpoolerPosition... Exiting...\n", taskId);
        exit(-1);
    }
    
    newJob.jobId = getpid();
    newJob.printTime = printTime;
    newJob.content = content;
    newJob.pages = pages;

    if(newJob.content != -1){
        printf(APP"Created\n",taskId);
        printf(APP"printTime: %d\n",taskId, newJob.printTime);
        printf(APP"content: %d\n",taskId, newJob.content),
        printf(APP"pages: %d\n",taskId, newJob.pages);
    }

    if(semop(semSpoolerEmpty, &w, 1) == -1){ //wait(spoolerEmpty)
        printf(APP"ERROR wait(spoolerEmpty)... Exiting...\n)",taskId);
        exit(-1);
    }
    
    if(semop(mutexSpooler, &w, 1) == -1){ // wait(spooler)
        printf(APP"ERROR wait(spooler)... Exiting...\n)",taskId);
        exit(-1);
    }  

    //begin critical area
    queue[*queuePosition] = newJob;
    (*queuePosition)++;
    //end critical area

	if(semop(mutexSpooler, &s, 1) == -1){ // signal(spooler)
        printf(APP"ERROR signal(spooler)... Exiting...\n)",taskId);
        exit(-1);
    }
    
    if(semop(semSpoolerFull, &s, 1) == -1){ //signal(spoolerFull)
        printf(APP"ERROR signal(spoolerFull)... Exiting...\n)",taskId);
        exit(-1);
    }

    shmdt((void*) queue);
    shmdt((void*) queuePosition);

}

void spooler(){

    job* spooler; 
    int* spoolerPosition; 
    job* printerQueue; 
    job nextJob;
    int printer;
    struct sembuf w;
    struct sembuf s;

    //wait
    w.sem_num = 0;
    w.sem_op = -1;
    w.sem_flg = 0;

    //signal
    s.sem_num = 0;
    s.sem_op =  1;
    s.sem_flg = 0;

    printer = 0; //control variable for switching printers

    if((spooler = shmat(shmSpoolerQueue, NULL, 0)) == (void*) -1){
        printf(SPOO"ERROR getting shmSpoolerQueue\n");
        exit(-1);
    }
    if((spoolerPosition = shmat(shmSpoolerPosition, NULL, 0)) == (void*) -1){
        printf(SPOO"ERROR getting shmSpoolerPosition\n");
        exit(-1);
    }
    if((printerQueue = shmat(shmPrinterQueue, NULL, 0)) == (void*) -1){
        printf(SPOO"ERROR getting shmPrinterQueue\n");
        exit(-1);
    }

    while(1){
        w.sem_num = 0;
        s.sem_num = 0;
        
        if(semop(semSpoolerFull, &w, 1) == -1){ //wait(spoolerFull)
            printf(SPOO"ERROR wait(spoolerFull)\n");
            exit(-1);
        }
        if(semop(mutexSpooler, &w, 1) == -1){ //wait(spooler)
            printf(SPOO"ERROR wait(spooler)\n");
            exit(-1);
        }
        //critical area taking from spoolqueue
        nextJob = spooler[0];
        //no more jobs
        if(nextJob.content == -1){
            
            semop(semPrinterEmpty, &w, 1); //wait(printerEmpty)
            printf(SPOO"Adding empty dummy job to end printer 0\n");
            printerQueue[0] = nextJob;
            semop(semPrinterFull, &s, 1); //signal(printerFull);

            w.sem_num = 1;
            s.sem_num = 1;
            
            semop(semPrinterEmpty, &w, 1); //wait(printerEmpty)
            printf(SPOO"Adding empty dummy job to end printer 1\n");
            printerQueue[1] = nextJob;
            semop(semPrinterFull, &s, 1); //signal(printerFull);            

            break;
        }

        for(int i = 0; i < *spoolerPosition; i++){
            spooler[i] = spooler[i+1];
        }
        printf(SPOO"Spooler added: %d\n", nextJob.jobId );
        printf(SPOO"print time: %d\n", nextJob.printTime );
        printf(SPOO"content: %d\n", nextJob.content );
        printf(SPOO"pages: %d\n", nextJob.pages );

        (*spoolerPosition)--;
        //end critical area taking from spool queue
        if(semop(mutexSpooler, &s, 1) == -1){ //signal (mutex spooler)
            printf(SPOO"ERROR signal(spooler)\n");
            exit(-1);
        }        
        if(semop(semSpoolerEmpty, &s, 1) == -1){ //signal (spoolerEmpty)
            printf(SPOO"ERROR signal(spoolerEmpty)\n");
            exit(-1);
        }   
        //put into printer queue
        w.sem_num = printer;
        s.sem_num = printer;

        if(semop(semPrinterEmpty, &w, 1) == -1){ //wait(printerEmpty)
            printf(SPOO"ERROR wait(printerEmpty)\n");
            exit(-1);
        }   
        printerQueue[printer] = nextJob;
        if(semop(semPrinterFull, &s, 1) == -1){ //signal(printerFull);
            printf(SPOO"ERROR signal(printerFull)\n");
            exit(-1);
        }   

        printf(SPOO"Spooler sent %d to printer %d\n", nextJob.jobId, printer);
        printf(SPOO"print time: %d\n", nextJob.printTime );
        printf(SPOO"content: %d\n", nextJob.content );
        printf(SPOO"pages: %d\n", nextJob.pages );

        printer = ++printer % PRINTERS;

    }

    printf(SPOO"Ending...\n");

    shmdt((void*) spooler);
    shmdt((void*) spoolerPosition);
    shmdt((void*) printerQueue);

}

void printer(int printerNr){

    job* printerQueue; 
    job toPrint;
    struct sembuf w;
    struct sembuf s;

    //wait
    w.sem_num = printerNr;
    w.sem_op = -1;
    w.sem_flg = 0;
    
    //signal
    s.sem_num = printerNr;
    s.sem_op =  1;
    s.sem_flg = 0;

    if((printerQueue = shmat(shmPrinterQueue, NULL, 0)) == (void*) -1){
        printf(PRINT"ERROR getting shmPrinterQueue\n", printerNr);
        exit(-1);
    }

    while(1){

        if(semop(semPrinterFull, &w, 1) == -1){ //wait(printerFull)
            printf(PRINT"ERROR wait(printerFull)\n", printerNr);
            exit(-1);
        }
        toPrint = printerQueue[printerNr];

        //no more jobs
        if(toPrint.content == -1){
            printf(PRINT"Found empty dummy job...\n", printerNr);
            semop(semPrinterEmpty, &s, 1); //signal(printerEmpty)
            break;
        }

        printf(PRINT"Starting job %d\n", printerNr, toPrint.jobId);
        printf(PRINT"content: %d\n", printerNr, toPrint.content);
        printf(PRINT"pages: %d\n", printerNr, toPrint.pages);
        printf(PRINT"duration: %d\n", printerNr, toPrint.printTime);
        sleep(printerQueue[printerNr].printTime); //print()
        if(semop(semPrinterEmpty, &s, 1) == -1){ //signal(printerEmpty)
            printf(PRINT"ERROR signal(printerEmpty)... Exiting...\n", printerNr);
            exit(-1);
        }
        
        printf(PRINT"Job %d finished\n", printerNr, toPrint.jobId);

    }

    printf(PRINT"Ended...\n", printerNr);

    shmdt((void*)printerQueue);

}
