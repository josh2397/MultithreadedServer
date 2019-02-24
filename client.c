#include "serverClient.h"



/*
    set up thread_pool function
    set up sempahore class
    convert number to long function

    Files:
        client
        

*/





void *createSharedMemory(size_t size) {
    void *shem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    // if (shem = (void *)-1) {
    //     perror("mmap error");
    // }
    return shem;
}

// void client(number) {
    


// }


void printSlots() {
    printf("printing slots");
}


int main(int argc, char *argv[]) {

    struct SharedMemory sharedMemory;

    sharedMemory.shutdown = (int *)createSharedMemory(sizeof(int));

    pthread_mutex_t clientFlagMutex;
    pthread_mutex_init(&clientFlagMutex, NULL);
    pthread_cond_t clientFlagCondition;
    pthread_cond_init(&clientFlagCondition, NULL);

    pthread_mutex_init(&printReadyMutex, NULL);
    pthread_cond_init(&printReady, NULL);
    
    
    sharedMemory.clientFlagMutex = clientFlagMutex;
    sharedMemory.clientFlagCondition = clientFlagCondition;

    

    
    sharedMemory.number  = (unsigned int*)createSharedMemory(sizeof(unsigned int));
    sharedMemory.clientFlag = (char*)createSharedMemory(sizeof(char));
    sharedMemory.serverFlag = (char*)createSharedMemory(sizeof(char) * 10);
    sharedMemory.slots = (int*)createSharedMemory(sizeof(int)*10);
    sharedMemory.numThreads = (int*)createSharedMemory(sizeof(int));

    *sharedMemory.shutdown = 0;
    if (argc == 2){
        *sharedMemory.numThreads = atoi(argv[1]);
    }
    else if (argc == 1) 
    {
        *sharedMemory.numThreads = 320;
    }
    *sharedMemory.clientFlag = '0';

    for (int i = 0; i < 10; i++) {
        sharedMemory.serverFlag[i] = '0';
        printf("server Flag %d : %c\n", i, sharedMemory.serverFlag[i]);
    }
    
    


    //pthread_cond_init(&clientFlagCondition, NULL);
    //pthread_mutex_lock(&clientFlagMutex);
    pthread_t serverThread;
    pthread_t printThread;
    //shem = (unsigned long)shem;
    pthread_attr_t tattr;
    int ret;
    ret = pthread_attr_init(&tattr);
    ret = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    
    ret = pthread_create(&serverThread, &tattr, server, (void *) &sharedMemory);

    ret = pthread_create(&printThread, &tattr, printFunction, (void *) &sharedMemory);
    


    char buffer[BUFLEN];
    printf("Enter an integer to be factored: ");
    //pthread_mutex_lock(&clientFlagMutex);

    // Get client input
    while (fgets(buffer, BUFLEN, stdin) != NULL) {

        char delim[5] = " \n\0";

        char *commands[64];
        int k = 0;
        int flag = 0;
        // Seperate the input into an array of commands
        commands[k] = strtok(buffer, delim);
        
        while( commands[k] != NULL ) {
            k++;
            commands[k] = strtok(NULL, delim);
        }

        

        if (strcmp(commands[0], "q") == 0) {
            *sharedMemory.shutdown = 1;
            printf("Quitting Application\n\nServer shutdown\n");
            exit(0);
        }
        // while (*sharedMemory.clientFlag == '1') {
        //     sleep(1);
        // }

        *sharedMemory.number = strtol(buffer, NULL, 10);
        //printf("\nNumber: %u\n", *sharedMemory.number);
        //memcpy(shem, number, sizeof(number));
        *sharedMemory.clientFlag = '1';
        
        pthread_cond_signal(&sharedMemory.clientFlagCondition);
        
        //pthread_mutex_unlock(&sharedMemory.clientFlagMutex);

        // while (*sharedMemory.clientFlag == '1'){
        //     pthread_cond_wait(&sharedMemory.clientFlagCondition, &sharedMemory.clientFlagMutex);
        // }
    }
    *sharedMemory.shutdown = 1;
    
    // void* clientFlag = createSharedMemory(sizeof(int));

    // //void* dispatchQueue = createSharedMemory(sizeof());

    // //stringToLong(number);
    // number = strtol()
    // printf("number: ", number);

    // client(number);
    
    // get number input

    // convert number to long

    // check if the number = 0 -> run test

    // create semaphore instance

    // create a new thread for server

    // 

}

void printFunction(struct SharedMemory *sharedMemory) {
    for (;;){
        for (int i = 0; i < 10; i++) {
                pthread_mutex_lock(&printReadyMutex);
                pthread_cond_wait(&printReady, &printReadyMutex);				
                //pthread_cond_signal(&printReady);
                pthread_mutex_unlock(&printReadyMutex);	
                //printf("ServerFlag Cleint Side %c\n", sharedMemory->serverFlag[i]);
                if (sharedMemory->serverFlag[i] == '1') {
                    printf("slot : %d - factor : %d\n", i, sharedMemory->slots[i]);
                    //fflush(stdout);
                    sharedMemory->serverFlag[i] = '0';
                    sharedMemory->slots[i] = 0;
                }

        }
    }
        
}

