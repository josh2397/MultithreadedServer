#include "serverClient.h"

void testMode(char *serverFlag, int *slots){
    pthread_t *testThreads = (pthread_t *)malloc(30 * sizeof(pthread_t) );
    pthread_attr_t tattr;
    int ret = pthread_attr_init(&tattr);
    ret = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    int index;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            struct ThreadPool testPool;
            testPool.number = j * 10;
            testPool.slots = slots;
            testPool.serverFlag;
            index = i*j;
            pthread_create(&testThreads[index], &tattr, testThread, (void *)&testPool);
        
        }
    }
        
}

void * testThread(void *args) {
    struct ThreadPool *testPool = args;
    printf("number passed to thread %d\n", testPool->number);
    for (int i = testPool->number; i < testPool->number + 10; i++){
        printf("index %d" , i);
        while (testPool->serverFlag[0] == 1) {
            pthread_mutex_lock(&printReadyMutex);
            pthread_cond_signal(&printReady);
            printf("sent signal\n");
            pthread_cond_wait(&printReady, &printReadyMutex);
            pthread_mutex_unlock(&printReadyMutex);
        }
        testPool->slots[0] = i;
        testPool->serverFlag[0] = 1;

    }
}

// void* testThread(void* args){
// 	struct TestArgs* ta = args;
// 	int* serverFlag = ta->serverFlag;
// 	int* slots = ta->slots;
// 	int startNum = ta->startNum;
// 	for (int i = startNum; i < startNum + 10; i++){
// 		while (serverFlag[0] == 1){
// 			pthread_mutex_lock(&printReadyMutex);
// 			pthread_cond_signal(&printReadyCond);
// 			//pthread_cond_wait(&printReadyCond, &printReadyMutex);
// 			pthread_mutex_unlock(&printReadyMutex);
// 		}
// 		slots[0] = i;
// 		serverFlag[0] = 1;
// 	}
// }

// void testMode(int* serverFlag, int* slots){
// 	printf("TEST MODE\n");
// 	pthread_t* threadPool = malloc(30 * sizeof(pthread_t));
	
	
// 	for (int i = 0; i < 3; i++){
// 		for (int j = 0; j < 10; j++){
// 			struct TestArgs ta;
// 			ta.startNum = j * 10;
// 			ta.slots = slots; 
// 			ta.serverFlag = serverFlag;
// 			pthread_create(&threadPool[i], NULL, testThread, (void*) &ta);
// 			sleep(1);
// 		}
// 	}
// }


void * server(void *arguments) {
    struct SharedMemory * sharedMemory = (struct SharedMemory *)arguments;

    pthread_cond_init(&wakeupThread, NULL);
    pthread_mutex_init(&wakeupThreadMutex, NULL);
    pthread_cond_init(&threadComplete, NULL);
    pthread_mutex_init(&threadCompleteMutex, NULL);
    //pthread_cond_init

    //wakeupThreadMutex = PTHREAD_MUTEX_INITIALIZER;
    struct ThreadPool threadPool;
    
    pthread_t *threads = (pthread_t *)malloc(*sharedMemory->numThreads * sizeof(pthread_t) );

    Queue* nQueue = ConstructQueue(320);
    threadPool.workQueue = nQueue;
    threadPool.serverFlag = sharedMemory->serverFlag;
    threadPool.slots = sharedMemory->slots;
    threadPool.runningThreads = 0;
    for (int i = 0; i < *sharedMemory->numThreads; i++) {
        pthread_attr_t tattr;
        int ret;
        ret = pthread_attr_init(&tattr);
        ret = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
        
        ret = pthread_create(&threads[i], &tattr, dispatchQueue, (void *) &threadPool);
    }

    
    
    
    while(1) {
        
        //int testFlag = 0;
        
        pthread_mutex_lock(&sharedMemory->clientFlagMutex);
        printf("clientFlag: %c\n", *sharedMemory->clientFlag);
        
        
        int ret = pthread_cond_wait(&sharedMemory->clientFlagCondition, &sharedMemory->clientFlagMutex);
        unsigned int number = *sharedMemory->number;
        
        pthread_mutex_unlock(&sharedMemory->clientFlagMutex);
        // Run test mode if true
        if (number == 0) {
            
            printf("Number of equals zero, entering test mode\n");
            testMode(sharedMemory->serverFlag, sharedMemory->slots);
            
        }
        if (ret != 0) {
            printf("pthread_cond_wait error: %d\n", ret);
        }
        

        if (*sharedMemory->number > 0) {
            //threadPool.number = sharedMemory->number;

            // create a queue node and find a slot 
            NODE *pN;
            NODE *slotIndex;
            slotIndex = (NODE*) malloc(sizeof(NODE));
            slotIndex->num = findSlot(sharedMemory->serverFlag);

            if (slotIndex->num == -1) {
                printf("Maximum number of queries are running concurrently\n");

            }

            *sharedMemory->clientFlag = '0';
            // Rotate the number 32 times and the jobs to the queue
            
            for (int i = 0; i < 32; i++) {
            
            
            
                pN = (NODE*) malloc(sizeof (NODE));
                // Place the number into the queue
                pN->num = number;
                number = (number >> 1) | (number << CHAR_BIT*sizeof(number) - 1);
                
                Enqueue(threadPool.workQueue, pN);
                Enqueue(threadPool.workQueue, slotIndex);
                //printf("Queue size: %d\n", threadPool.workQueue->size);
                //printf("Queued job: %d\n", i);
                //printf("queued number: %u\n", pN->num);
                
                if (threadPool.runningThreads >= *sharedMemory->numThreads) {
                    printf("Server at capacity\n");
                    pthread_mutex_lock(&threadCompleteMutex);
                    pthread_cond_wait(&threadComplete, &threadCompleteMutex);
                    pthread_mutex_unlock(&threadCompleteMutex);
                }
                
                pthread_mutex_lock(&wakeupThreadMutex);
                threadPool.runningThreads++;
                printf("Running threads %d\n", threadPool.runningThreads);
                pthread_cond_signal(&wakeupThread);
                pthread_mutex_unlock(&wakeupThreadMutex);

                pthread_mutex_lock(&threadCompleteMutex);
                
                pthread_cond_wait(&threadComplete, &threadCompleteMutex);
                pthread_mutex_unlock(&threadCompleteMutex);
                //sleep(0.1);

            }
        }

        
    }
    
}



void dispatchQueue(struct ThreadPool *threadPool) {
    pthread_mutex_t factorLock;
    //printf("Thread alive\n");
    while (1){

        pthread_mutex_lock(&wakeupThreadMutex);
        
        pthread_cond_wait(&wakeupThread, &wakeupThreadMutex);
        pthread_mutex_unlock(&wakeupThreadMutex);
        //printf("Entered thread function\n");
        //printf("queue number: %u\n", (*threadPool).workQueue->head->num);
        

        //pthread_mutex_lock(&factorLock);
        NODE *Num = Dequeue((*threadPool).workQueue);
        NODE *slotIndex = Dequeue((*threadPool).workQueue);
        // if (Num->num == NULL) {

        // }
        //pthread_mutex_unlock(&factorLock);
        printf("Factorising %u\n",Num->num);
        factorise(Num->num, slotIndex->num, threadPool->serverFlag, threadPool->slots);
        
   
        
        
        pthread_mutex_lock(&threadCompleteMutex);
        threadPool->runningThreads--;
        pthread_cond_signal(&threadComplete);
        pthread_mutex_unlock(&threadCompleteMutex);

    }
    
}


// Get the slot index
void factorise(int number, int slotIndex, char *serverFlag, int *slots) {
    printf("ServerFlag %c\n", serverFlag[slotIndex]);
    
    for (int i = 1; i < (number/2); i++) {
       
        int result = number % i;
        if (result == 0) {
            //printf("Factor: %d\n", i);
            while(serverFlag[slotIndex] == '1'){
                //printf("serverFlag %c\n", serverFlag[slotIndex]);
                pthread_mutex_lock(&printReadyMutex);
                pthread_cond_signal(&printReady);
                //pthread_cond_wait(&printReady, &printReadyMutex);
                //printf(" ");
                pthread_mutex_unlock(&printReadyMutex);
            }
            //printf("%d", slotIndex);
            slots[slotIndex] = i;
            serverFlag[slotIndex] = '1';
             
            /*
            -------------------------------------------------------------
            Fix Seg fault with possibly serverFlag and slots assignment!
            Synchronising serverFlag and slots messages between server and client
            -------------------------------------------------------------
            */
            

        }
    }
    //printf("DOOOONEEEN %u\n",number);
}




int findSlot(char *serverFlags) {
    for (int i = 0; i < 10; i++) {
        if (serverFlags[i] == '0') {
            return i;
        }
    }
    return -1;

}