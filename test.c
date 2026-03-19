//import headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

//define macros
#define PORT 9000
#define BUFFER 1024
#define MAX_WORKERS 5
#define MAX_QUEUE 5

//define task linked list (client FD)
typedef struct task {
    int client_fd
    task* next
} task_t;

//define thread pool
typedef tread_pool {
    pthread_t* thread_pool[MAX_WORKERS]
    task_t* head
    task_t* tail
    pthread_mutex_t lock
    pthread_cond_t  notify
    int q_size
    int stop
} thread_pool_t;


//define thread pool init

thread_pool_t pool

void* worker_thread(void* arg){
    while(1){
    
        pthread_mutex_lock(&pool.lock);

        if (pool.head==NULL & !pool.stop){
            pthread_cond_wait(&pool.notify, &pool.lock);
        }
        if (pool.stop){
            pthread_mutex_unlock(&pool.lock);
            return;
        }

        //DEQUEUE 
        task_t* task = pool.head;
        pool.head = task->next;

        if (pool.head==NULL){
            pool.tail=NULL;
        }

        pool.q_size--;
        printf("[WORKER] picked up client fd - %d\n",task->client_fd);
        pthread_mutex_unlock(pool.lock);
        handle_client(task->client_fd);
        free(task);
    }
    return NULL;
}


void thread_pool_init(){
    pool.head = NULL;
    pool.tail = NULL;
    pool.q_size = 0;
    pool.stop = 0;
    for (int i = 0; i< MAX_WORKERS;i++){
        pthread_create(&thread_pool[i], NULL, worker_thread,NULL);
    }
}
//define add_task_to_pool enqueue
//define worker_function dequeue
//define handle_client
