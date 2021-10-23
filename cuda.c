#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <unistd.h>
#include <pthread.h>

// Modified from CrackAZ99.c

int found = 0;

typedef struct crackInfo crackInfo;
struct crackInfo{
  char* salt_and_encrypted;
  int start; // which z value to start at
  int step; // aka the z step value, for n number of threads upto 99, mark scheme says many threads not infinite.
  //start will be i from the pthreade_create(...) loop
  //step will be the number of threads.
};

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}

void *crack(void *voidPtr){
  int x, y, z;     // Loop counters
  char salt[7];    
  char plain[7];   
  char *enc;
  struct crypt_data data;
  data.initialized = 0;
  
  struct crackInfo *myDataPtr = (struct crackInfo*)voidPtr;

  substr(salt, myDataPtr->salt_and_encrypted, 0, 6);

  for(x='A'; x<='Z'; x++){
    for(y='A'; y<='Z'; y++){
      for(z=myDataPtr->start; z<=99; z=z+myDataPtr->step){
        switch(found){
          case 0://not been found yet
            sprintf(plain, "%c%c%02d", x, y, z); 
            enc = (char *) crypt_r(plain, salt, &data);
            printf("Attempting: %s\n", plain);
            if(strcmp(myDataPtr->salt_and_encrypted, enc) == 0){ //found the match.
             printf("The original password was: %s\n", plain);
             found = 1; //let the other threads know we found the password.
  	     pthread_exit(0);
            }
            break;
          case 1://another thread found the password
            printf("Shutting down thread: %ld\n", pthread_self());
            pthread_exit(NULL);   
            
        }
      }
    }
  }
    //this thread didn't find it
    pthread_exit(NULL);     
}


void main(int argc, char *argv[]){
  char* argvEncrypted;
  int numOfThreads;
  
  if(argc = 3){// 0 is file name, 1 is number of threads, 2 is encrypted password
    if(atoi(argv[1])> 0 && atoi(argv[1])<100){
      numOfThreads = atoi(argv[1]);
    }else{
      printf("Maximum number of threads capped at 99! Not %s\n", argv[1]);
      numOfThreads = 99;
    }
    
    argvEncrypted = argv[2];
    
    crackInfo* args = malloc(sizeof(*args)*numOfThreads); // create a struct for each thread
    pthread_t* threads = malloc(sizeof(pthread_t)*numOfThreads);
    
    for(int i = 0; i < numOfThreads; i++){//split the task up over i threads
      args[i].salt_and_encrypted = argvEncrypted;
      args[i].start = i;    	
      args[i].step = numOfThreads;
      pthread_create(&threads[i], NULL, crack, &args[i]);
    }
    //need to join, so that we have printf the password.
    for(int y = 0; y < numOfThreads; y++){
      pthread_join(threads[y], NULL);
    }
    printf("Main complete.\n");
    pthread_exit(0);
  }else{
    printf("Please give the second argv as the number of threads and the third as the encrypted password.\n");
  }
}
