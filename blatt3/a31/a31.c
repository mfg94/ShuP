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
int shmSpoolerQueue;
int shmSpoolerPosition;
int shmPrinterQueue;

int semSpoolerFull;
int semSpoolerEmpty;
int mutexSpooler;
int semPrinterFull;
int semPrinterEmpty;

void spooler();
void app(int printTime, int content, int pages);
void printer(int printerNr);

#define QUEUE_SIZE 5
#define PRINTERS 2

int main(int argc, char** argv){

	pid_t spoolerPid;
	pid_t taskPid;
	pid_t printerPid[PRINTERS];
    char* endptr;
	int numberOfTasks;
	int printTime;
	int content;
	int pages;
	int taskExitStatus;
	int waitTime;
	int i;
	int retVal;

	printf("Diese Loesung wurde erstellt von Michael Gutmair\n");

	if((argc <=1) || (numberOfTasks = strtol(argv[1], &endptr, 10)) <= 0){
        printf(MAIN"Please provide an integer as first argument!\n");
        return -1;
    }

	//initialize random
	srand(time(NULL));

    //initialize semaphores and shared memory
    shmSpoolerQueue = shmget(IPC_PRIVATE, QUEUE_SIZE * sizeof(job), 0777 | IPC_CREAT); //array for job queue

    shmSpoolerPosition = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT); //akt. position in job queue
   	int* spoolerPosition = shmat(shmSpoolerPosition, NULL, 0);
   	*spoolerPosition = 0; //set index pos to 0

   	shmPrinterQueue = shmget(IPC_PRIVATE, PRINTERS * sizeof(job), 0777 | IPC_CREAT); //array for printer queue

    semSpoolerFull = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //sem queueFull
   	semctl(semSpoolerFull, 0, SETVAL, 0);//sem queuefull = 0

    semSpoolerEmpty = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //sem queueEmpty
   	semctl(semSpoolerEmpty, 0, SETVAL, QUEUE_SIZE);//sem queueEmpty = 5

    mutexSpooler = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //mutex queue
	semctl(mutexSpooler, 0, SETVAL, 1);   //mutex queue = 1

	semPrinterFull = semget(IPC_PRIVATE, PRINTERS, 0777 | IPC_CREAT); //sem printerFull
	semctl(semPrinterFull, 0, SETVAL, 0); //sem printerFull[0] = 0
	semctl(semPrinterFull, 1, SETVAL, 0); //sem printerFull[1] = 0

	semPrinterEmpty = semget(IPC_PRIVATE, PRINTERS, 0777 | IPC_CREAT); //sem printerEmpty
	semctl(semPrinterEmpty, 0, SETVAL, 1); //sem printerEmpty[0] = 1
	semctl(semPrinterEmpty, 1, SETVAL, 1); //sem printerEmpty[1] = 1

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

        waitpid(taskPid, &taskExitStatus, 0);

	}

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
	shmctl(semSpoolerEmpty, IPC_RMID, NULL);
    shmctl(semSpoolerFull, IPC_RMID, NULL);
    shmctl(mutexSpooler, IPC_RMID, NULL);
    shmctl(semPrinterFull, IPC_RMID, NULL);
    shmctl(semPrinterEmpty, IPC_RMID, NULL);

	return 0;

}


void app(int printTime, int content, int pages){

    job* queue = shmat(shmSpoolerQueue, NULL, 0);
    int* queuePosition = shmat(shmSpoolerPosition, NULL, 0);
    int taskId = getpid();

    struct sembuf w;
    w.sem_num = 0;
    w.sem_op = -1; //wait
    w.sem_flg = 0;

    struct sembuf s;
    s.sem_num = 0;
    s.sem_op =  1; //signal
    s.sem_flg = 0;

    job newJob = {.jobId = getpid(), .printTime = printTime, .content = content, .pages = pages};

    if(newJob.content != -1){
        printf(APP"Created\n",taskId);
        printf(APP"printTime: %d\n",taskId, newJob.printTime);
        printf(APP"content: %d\n",taskId, newJob.content),
        printf(APP"pages: %d\n",taskId, newJob.pages);
    }

    semop(semSpoolerEmpty, &w, 1); //wait(queueEmpty)
    //kritischer Bereich
    semop(mutexSpooler, &w, 1); // wait(mutex queue)
    queue[*queuePosition] = newJob;
    (*queuePosition)++;
	semop(mutexSpooler, &s, 1); // signal(mutex queue)
    //Ende kritischer Bereich
    semop(semSpoolerFull, &s, 1); //signal(queueFull)

    shmdt((void*) queue);
    shmdt((void*) queuePosition);

}

void spooler(){

    job* queue = shmat(shmSpoolerQueue, NULL, 0);
    int* queuePosition = shmat(shmSpoolerPosition, NULL, 0);
    job* printerQueue = shmat(shmPrinterQueue, NULL, 0);

    int printer = 0; //control variable for printer

    struct sembuf w;
    w.sem_num = 0;
    w.sem_op = -1; //wait
    w.sem_flg = 0;

    struct sembuf s;
    s.sem_num = 0;
    s.sem_op =  1; //signal
    s.sem_flg = 0;

    while(1){
        w.sem_num = 0;
        s.sem_num = 0;

        //kritischer Bereich
        semop(semSpoolerFull, &w, 1);//wait(queueFull)
        semop(mutexSpooler, &w, 1); //wait(mutex)
        job nextJob = queue[0];

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

        for(int i = 0; i < *queuePosition; i++){
            queue[i] = queue[i+1];
        }
        printf(SPOO"Spooler added: %d\n", nextJob.jobId );
        printf(SPOO"print time: %d\n", nextJob.printTime );
        printf(SPOO"content: %d\n", nextJob.content );
        printf(SPOO"pages: %d\n", nextJob.pages );

        (*queuePosition)--;

        semop(mutexSpooler, &s, 1); //signal (mutex spooler)
        semop(semSpoolerEmpty, &s, 1); //signa (queueEmpty)
        //Ende kritischer Bereich


        w.sem_num = printer;
        s.sem_num = printer;
        semop(semPrinterEmpty, &w, 1); //wait(printerEmpty)
        printerQueue[printer] = nextJob;
        printer = ++printer % PRINTERS;
        semop(semPrinterFull, &s, 1); //signal(printerFull);

        printf(SPOO"Spooler sent %d to printer %d\n", nextJob.jobId, printer);
        printf(SPOO"print time: %d\n", nextJob.printTime );
        printf(SPOO"content: %d\n", nextJob.content );
        printf(SPOO"pages: %d\n", nextJob.pages );



    }

    printf(SPOO"Ending...\n");

    shmdt((void*) queue);
    shmdt((void*) queuePosition);
    shmdt((void*) printerQueue);

}

void printer(int printerNr){

    job* printerQueue = shmat(shmPrinterQueue, NULL, 0);

    struct sembuf w;
    w.sem_num = printerNr;
    w.sem_op = -1; //wait
    w.sem_flg = 0;

    struct sembuf s;
    s.sem_num = printerNr;
    s.sem_op =  1; //signal
    s.sem_flg = 0;


    while(1){

        semop(semPrinterFull, &w, 1);//wait(printerFull)
        job toPrint = printerQueue[printerNr];

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
        sleep(printerQueue[printerNr].printTime);
        semop(semPrinterEmpty, &s, 1); //signal(printerEmpty)
        printf(PRINT"Job %d finished\n", printerNr, toPrint.jobId);

    }

    printf(PRINT"Ended...\n", printerNr);

    shmdt((void*)printerQueue);

}
