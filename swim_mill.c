#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/sem.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

//Define the size of the 2D array
#define col 10
#define row 10

//a function that displays the 2D array representation of sea shore and for terminate and interrup, sempahore controls
void display();
void deallocateSHM();
void exitInterupt();
void exitProcess();
void startSemaphore();
void endSemaphore();
void unlinkSemaphore();


int shm_id;

//a key to access the specific block of shared memory
#define shm_key_t 2323
#define SEMAPHORE "/semaphore"
//process id for all processes - signed int
pid_t swim_mill;
pid_t pellet;
pid_t fish;

// pointers to the shared memory 2D array
char (*seashore)[row][col];

sem_t *sem; 

//file for compilation
FILE *f_compilation;

int main(){
	
    printf("PID %d Swim Mill started...\n", getpid());
    
    //interupt(^C) and exit signal handler handler
    signal(SIGINT, exitInterupt); 
    signal(SIGTERM, exitProcess);
     
    shm_id = shmget(shm_key_t, row * col * sizeof(seashore), IPC_CREAT | 0666);
    if(shm_id < 0){
        perror("shmget failed....");
        //2 indicates its a misuse of builtin function
        exit(2);
    }
    //Assign the shm segment associated with the shared memory identifier specified by 
    //the variable shm id to the first accessible location as determined by the system as 
    //the second parameter is null pointer
    //On completion of the shmat function call, the start of the shared memory segment is assigned to shm_ptr.
    seashore = shmat(shm_id,NULL,0);

    if(seashore == (void*) -1){
        perror("shmat failed....");
        exit(2);
    }
    printf( "Shared memory (shm_id- %d) created. \n", shm_id);
    
    
    //2D array (seashore) every cell represented by -
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
              (*seashore)[i][j] = '-';
        }
        if(i == row - 1)
           (*seashore)[i][col/2] = 'F';
    }

     //open semaphore	 
    startSemaphore();
    
    //create fish process
    fish = fork();
    if(fish == 0){
        const char  *fish_arg = "";
         //recieving process fish execution 
        execl("./fish", fish_arg,NULL);
        exit(0);
    }
    else if(fish > 0){
    	//create pellet process
   	 pellet = fork();
   	 sleep(1);
    	 if(pellet == 0){
            const char  *pellet_arg = "";
            //recieving process pellet execution 
            execl("./pellet", pellet_arg,NULL);
            exit(0);
         }
         else {
                sleep(1);
		for (int time = 30; time >= 0; time--) {
	            sleep(1);
	            printf("%d seconds to quit... \n", time);
	            
   		    sem_wait(sem); //decreamnt the semaphore
		    display(seashore);
		    sem_post(sem); //increment the semaphore
	        }
         }
    }
    else {
    	 perror("Fork failed");
	 exit(3);	
    }
    exitProcess();
    return 0;
}

//print the state of the 2D shared memory segment helper
void display(char *sharedM){

    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            putchar( (*seashore)[i][j]);
            putchar(' ');
        }
        putchar('\n');
    }
    putchar('\n');
}

//exit process handler
void exitProcess(){
   printf("\nChildren processes being terminated and exiting swim mill...\n");
   
    //using SIGTERM - signal termination terminate Fish and Pellet pprocesses
   if(kill(fish, SIGTERM) == 0)
      printf("\nFish process (pid: %d) is killed...\n", fish); 
   
   if(kill(pellet, SIGTERM) == 0)
     printf("\nPellet process (pid: %d) is killed... \n", pellet);
   
   sleep(1);
   
   endSemaphore();
   unlinkSemaphore();
   deallocateSHM();
   printf("\nExiting swim_mill, pid: %d...\n", swim_mill);
   
   exit(0);
}

//deallocation of SMH handler
void deallocateSHM(){
   printf( "\nStarted deallocating the SHM...\n");
   
   //Deallocate the shared memory using shmdt
   if(shmdt(seashore) == 0)
      printf("\nDeallocating (shmid: %d)...\n", shm_id);
   
   //to remove shared memory segment @ shm_id
   if(shmctl(shm_id, IPC_RMID, 0) == 0)
      printf("\nShared memory segement (shmid: %d) removed...\n", shm_id);
}

//Interupt hundler
void exitInterupt()
{
   printf("\nInterrupted\n");
   sleep(1);
   exitProcess();
}
// Create a semaphore from a defined key if it doesn't already exist
void startSemaphore() {
    if ((sem = sem_open(SEMAPHORE, O_CREAT, 0644, 1)) == SEM_FAILED ) {
        printf("\nsem_open failure...\n");
        exit(2);
    }
    else
	printf("\n sem_open completed...\n");    
}

// Close a semaphore and deny access
void endSemaphore() {
    if (sem_close(sem) == -1) {
        printf("\nsem_close failure...\n");
        exit(2);
    }
    else
	printf("\n sem_close completed...\n");
}

// Remove semaphore from memory 
void unlinkSemaphore() {
	if (sem_unlink(SEMAPHORE) == -1) {
		printf("\nsem_unlink failure...\n");
		exit(2);
	}
	else
	    printf("\n sem_unlink completed...\n");
}


