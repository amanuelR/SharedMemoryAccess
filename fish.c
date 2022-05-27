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

void toRightOrLeft(int whichWay);
void deallocateSHM();
void exitProcess();
void exitInterupt();
void startSemaphore();
void endSemaphore();
void unlinkSemaphore();

int curr_location_col = col/2; 

int shm_id;

//a key to access the specific block of shared memory
#define shm_key_t 2323

//a key for semaphore 
#define SEMAPHORE "/semaphore"

// pointers to the shared memory 2D array
char (*seashore)[row][col];

sem_t *sem; 
//file for compilation
FILE *f_compilation;

int main(){

    //interupt(^C) and exit signal handler 
    signal(SIGINT, exitInterupt); 
    
    printf("PID %d Fish started \n", getpid());

             //location of the shared memory which was created in swim_mill
    shm_id = shmget(shm_key_t, row*col*sizeof(seashore), IPC_CREAT | 0666);
    if(shm_id < 0){
        perror("Fish shmget failed....");
        //2 indicates its a misuse of builtin function
        exit(2);
    }
              //point to the shared memorey
    seashore = shmat(shm_id,NULL,0);

    if(seashore == (void *) - 1){
        perror("Fish shmat failed....");
        exit(2);
    }
    printf("Shared memory fish access (shm_id- %d) granted... \n", shm_id);
    
    //open semaphore
    startSemaphore();
    
    while(1) {
    
	     //an algorithm to find all the pellets starting from the bottom row
	     //the 1D array holds 0s if there are no pellets in that row
	     //or it changes the 0 to 1 if there is a pellet in that row
	     
	     for(int r= row - 1; r >= 0; r--) {
               
                 // 1D arrow to find out where the pellets are
                 int currentRowPellets[col] = {0};
                
                 for (int c = 0; c < col; c++) {
                   
                    //check if there is a pellet at [r,c] then set that index to 1 if there is a pellet
                    if((*seashore)[r][c] == 'o' && !(currentRowPellets[curr_location_col] == 1) && abs(curr_location_col - c) <= abs(9 - r))  currentRowPellets[c] = 1;
                }
            
                // Don't move if pellet is coming at fish
                if((r == row - 2)&& currentRowPellets[curr_location_col] == 1) {
                    sleep(1);
                    break;
                }
                else {
                     
                     // locate the nearest pellet so that we prepare the next move of the fish
                     int nearestPellet = 0;
                     for(int c = 0; c < col; c++) {
                        if(currentRowPellets[c] == 1) {
                           nearestPellet = c;
                           break;
                        }
                     }
                
                     // If the fish can reach the pellet
                     if(nearestPellet != 0) {
                         // move the fish accordingly 
                         //toRight if the current position col is less than the closest pellet column
                         //toLeft if the current position col is greater than the closest pellet column
                         //or let the fish stays where it's at if the closest pellet is on the same column 
                         if(nearestPellet > curr_location_col) {
                            sem_wait(sem);  //decreamnt the semaphore
                            toRightOrLeft(1);
                            sem_post(sem);  //increment the semaphore
                         }
                         else if(nearestPellet == curr_location_col){
                            sem_wait(sem);  //decreamnt the semaphore
                            toRightOrLeft(0);
                            sem_post(sem);  //increment the semaphore
                         
                         }
                         else {
                              sem_wait(sem); //decreamnt the semaphore
                              toRightOrLeft(-1);
                              sem_post(sem); //increment the semaphore
                         }

                         sleep(1);
                         break;
                    }//end of if clause
               }//end of outer else
           }//end of for loop
       
    }//end of while loop

}
//update the previous position of fish to - and move the fish right or left based on the given direction
void toRightOrLeft(int whichWay) {
	

      (*seashore)[row-1][curr_location_col] = '-';
      //decrement or increment the cur_location_col of the fish according and set that location to be the cur_location_col
      curr_location_col += whichWay;
         
      (*seashore)[9][curr_location_col] = 'F';
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
   printf("\nFish Process Interrupted...\n");
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


