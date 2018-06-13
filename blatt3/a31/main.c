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

#define QUEUE_SIZE 5
#define PRINTERS 2

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
int shmDone;
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

int main(int argc, char** argv){

	pid_t spoolerPid;
	pid_t taskPid;
	pid_t printerPid[2];
	int numberOfTasks;
	int printTime;
	int content;
	int pages;
	int taskExitStatus;
	int waitTime;
	int i;
	int retVal;

	printf("Diese Loesung wurde erstellt von Michael Gutmair\n\n");

	numberOfTasks = atoi(argv[1]);

	//initialize random
	srand(time(NULL));

    //initialize semaphores and shared memory
    shmSpoolerQueue = shmget(IPC_PRIVATE, QUEUE_SIZE * sizeof(job), 0777 | IPC_CREAT); //array for job queue

    shmDone = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT); //control flag
    int* done = shmat(shmDone, NULL, 0); //set done to 0
    *done = 0;

    shmSpoolerPosition = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT); //akt. position in job queue
   	int* jobQueuePosition = shmat(shmSpoolerPosition, NULL, 0);
   	*jobQueuePosition = 0; //set index pos to 0

   	shmPrinterQueue = shmget(IPC_PRIVATE, PRINTERS * sizeof(job), 0777 | IPC_CREAT); //array for printer queue

    semSpoolerFull = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //sem queueFull
   	semctl(semSpoolerFull, 0, SETVAL, 0);//sem queuefull = 0

    semSpoolerEmpty = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //sem queueEmpty
   	semctl(semSpoolerEmpty, 0, SETVAL, QUEUE_SIZE);//sem queueEmpty = 5

    mutexSpooler = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT); //mutex queue
	semctl(mutexSpooler, 0, SETVAL, 1);   //mutex queue = 1

	semPrinterFull = semget(IPC_PRIVATE, 2, 0777 | IPC_CREAT); //sem printerFull
	semctl(semPrinterFull, 0, SETVAL, 0); //sem printerFull[0] = 0
	semctl(semPrinterFull, 1, SETVAL, 0); //sem printerFull[1] = 0

	semPrinterEmpty = semget(IPC_PRIVATE, 2, 0777 | IPC_CREAT); //sem printerEmpty
	semctl(semPrinterEmpty, 0, SETVAL, 1); //sem printerEmpty[0] = 1
	semctl(semPrinterEmpty, 1, SETVAL, 1); //sem printerEmpty[1] = 1

	//Start spooler
    if((spoolerPid = fork()) == 0){
        printf(MAIN"Spooler started...\n");
        spooler();
        printf(MAIN"Spooler ended...\n");
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
	printf(MAIN"Adding final empty job to end program...\n ");

    app(-1, -1, -1);

   	*done = 1;

	//end all waiting processes
	struct sembuf s;
    s.sem_num = 0;
    s.sem_flg = 0;
    s.sem_op = 1; //signal

    semop(semSpoolerFull, &s, 1); //signal (semSpoolerFull)
    semop(semSpoolerFull, &s, 1); //signal (semPrinterFull[0])
    s.sem_num = 1;
    semop(semSpoolerFull, &s, 1); //signal (semPrinterFull[1])


    printf(MAIN"Waiting for spooler to finish...\n");
	waitpid(spoolerPid, &taskExitStatus, 0);
    printf(MAIN"Spooler finished...\n");
    printf(MAIN"Waiting for printer 0 to finish...\n");
	waitpid(printerPid[0], &taskExitStatus, 0);
    printf(MAIN"Printer 0 finished\n");
    printf(MAIN"Waiting for printer 1 to finish...\n");
	waitpid(printerPid[1], &taskExitStatus, 0);
	printf(MAIN"Printer 0 finished\n");


	shmdt((void*) jobQueuePosition);
	shmdt((void*) done);

    //delete shared memory and semaphores
	shmctl(shmSpoolerQueue, IPC_RMID, NULL);
	shmctl(shmDone, IPC_RMID, NULL);
	shmctl(shmSpoolerPosition, IPC_RMID, NULL);
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

    printf(APP"Created\n",taskId);
    printf(APP"printTime: %d\n",taskId, newJob.printTime);
    printf(APP"content: %d\n",taskId, newJob.content),
    printf(APP"pages: %d\n",taskId, newJob.pages);

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
    int* done = shmat(shmDone, NULL, 0);
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
            printerQueue[0] = nextJob;
            semop(semPrinterFull, &s, 1); //signal(printerFull);
            printf(SPOO"Added final job to printer 0\n");

            w.sem_num = 1;
            s.sem_num = 1;

            semop(semPrinterEmpty, &w, 1); //wait(printerEmpty)
            printerQueue[1] = nextJob;
            semop(semPrinterFull, &s, 1); //signal(printerFull);
            printf(SPOO"Added final job to printer 1\n");

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
        semop(semPrinterFull, &s, 1); //signal(printerFull);

        printf(SPOO"Spooler sent %d to printer %d\n", nextJob.jobId, printer);
        printf(SPOO"print time: %d\n", nextJob.printTime );
        printf(SPOO"content: %d\n", nextJob.content );
        printf(SPOO"pages: %d\n", nextJob.pages );

        printer = ++printer % 2;


    }

    printf(SPOO"Ending...\n");

    shmdt((void*) queue);
    shmdt((void*) queuePosition);
    shmdt((void*) done);
    shmdt((void*) printerQueue);

}

void printer(int printerNr){

    int* done = shmat(shmDone, NULL, 0);
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
        if(printerQueue[printerNr].content == -1){
            printf(PRINT"Found final job...\n", printerNr);
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

    shmdt((void*) done);
    shmdt((void*)printerQueue);

}
