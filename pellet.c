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
#include <stdbool.h>

//Define the size of the 2D array

#define col 10
#define row 10

//key to access the specific block of shared memory
#define shm_key_t 2323
#define SEMAPHORE "/semaphore"

static void *child(void*);
void deallocateSHM();
void exitInterupt();
void exitProcess();
void startSemaphore();
void endSemaphore();
void unlinkSemaphore();

int curr_location_col; 

// pointers to the shared memory
char (*seashore)[row][col];

sem_t *sem; 

int shm_id;

//file for compilation
FILE *f_compilation;

int main(){

    //interupt(^C) and exit signal handler handler
    signal(SIGINT, exitInterupt); 
    
    printf("PID %d Pellet started\n", getpid());
    
             //location of the shared memory which was created in swim_mill
    shm_id = shmget(shm_key_t, sizeof(seashore), IPC_CREAT | 0666);
    if(shm_id < 0){
        perror("pellet shmget failed....");
        //2 indicates its a misuse of builtin function
        exit(2);
    }
              //point to the shared memorey
    seashore = shmat(shm_id,NULL,0);

    if(seashore == (void *) - 1){
        perror("pellet shmat failed....");
        exit(2);
    }
    
    printf("Shared memory pellet access (shm_id- %d) granted... \n", shm_id);
    
    //open semaphore
    startSemaphore();
    // a collection of pellet threads
    pthread_t pellet_threads[30];
    
    for(int i = 0; i < 30; i++) {
            // Create new pellet thread
            pthread_create(&pellet_threads[i], NULL, child, NULL);
            
            sleep(2);
    }
    
    sleep(1);
    
    // Continue when the thread list is empty
    pthread_join(pellet_threads[30], NULL);
    
    printf( "\nEND of Pellet process...%d\n", shm_id);
    exit(0);

}

static void *child(void* ignored) {
    
    // rrandom [row,col] position for pellet thread different from the other pellets
    int pelletRow = rand()%(row - 1);
    int pelletCol = rand()%(col - 1);
    
    while((*seashore)[pelletCol][pelletRow] == 'o'){
    	pelletRow = rand()%(row - 1);
    	pelletCol = rand()%(col - 1);
    }
    
    sem_wait(sem); //decreamnt the semaphore
    printf("Pellet %d was dropped at [%d,%d] ...\n", (int)pthread_self(), pelletRow, pelletCol);
    sem_post(sem); //increment the semaphore
    bool eaten = false;
    bool outOfReach= false;
    while(1) {
        
        //drop the pellet at a random position
        (*seashore)[pelletRow][pelletCol] = 'o';
        sleep(1);
        
        //move the pellet downstream
        pelletRow++;
        
        // Check if pellet was eaten or left stream
        if(pelletRow == row) outOfReach = true;
        
        //check if the current position of the pellet is replaced by 'F' which implies the pellet is eaten
        else if((*seashore)[pelletRow][pelletCol] == 'F') eaten = true;
        
        // if the pellet movement didn't lead it to leave the stream or be eaten by the fish
        // that means the pellet is moving forward so replace the pellet previous position by -
        if((*seashore)[pelletRow-1][pelletCol] != 'F') {
           
           sem_wait(sem); //decreamnt the semaphore
           (*seashore)[pelletRow-1][pelletCol] = '-';
           sem_post(sem); //increment the semaphore
        }
        
        //if the pellet is outOfReach or eaten break and print out the message accordingly
        if(eaten || outOfReach) break;
    }
    
    
    if(eaten) printf("Pellet %d was eaten at column %d...\n", (int)pthread_self(), pelletCol);
    else printf("Pellet %d wasn't eaten and left stream at column %d ...\n", (int)pthread_self(), pelletCol);
    sleep(1);
    
    
    return NULL;
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
   printf("\nPellet Process Interrupted\n");
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



