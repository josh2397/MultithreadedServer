#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define BUFLEN 64

// typedef struct DATA;
// typedef struct node_t NODE;
// typedef struct Queue Queue;
/* a link in the queue, holds the info and point to the next Node*/
typedef struct {
    int info;
} DATA;

typedef struct Node_t {
    int num;
    struct Node_t *prev;
} NODE;

/* the HEAD of the Queue, hold the amount of node's that are in the queue*/
typedef struct Queue {
    NODE *head;
    NODE *tail;
    int size;
    int limit;
} Queue;

struct SharedMemory {
    unsigned int* number;
    char *clientFlag;
    char *serverFlag;
    int *slots;
    int *numThreads;
    int *shutdown;

    pthread_mutex_t clientFlagMutex;
    pthread_cond_t clientFlagCondition;

};

struct ThreadPool {
    Queue *workQueue;
    int runningThreads;
    char *serverFlag;
    int *slots;
    int number;

};
struct TestArgs{
	int startNum;
	int* serverFlag,* slots;
};

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

// 

pthread_cond_t wakeupThread;
pthread_mutex_t wakeupThreadMutex;

pthread_cond_t threadComplete;
pthread_mutex_t threadCompleteMutex;

pthread_cond_t printReady;
pthread_mutex_t printReadyMutex;

pthread_mutex_t testMutex;
pthread_cond_t testCond;


void *createSharedMemory(size_t size);
void *server(void *arguments);
void dispatchQueue(struct ThreadPool *threadPool);
int findSlot(char *serverFlags);
void factorise(int number, int slotIndex, char *serverFlag, int *slots);
void printFunction(struct SharedMemory *sharedMemory);

void * testThread(void *args);
void testMode(char *serverFlag, int *slots);

//void *client();





Queue *ConstructQueue(int limit);
void DestructQueue(Queue *queue);
int Enqueue(Queue *pQueue, NODE *item);
NODE * Dequeue(Queue *pQueue);
int isEmpty(Queue* pQueue);