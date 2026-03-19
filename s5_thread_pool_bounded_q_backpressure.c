#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 9000
#define BUFFER 1024
#define THREAD_COUNT 4
#define MAX_QUEUE 5   // 🔥 Backpressure limit

typedef struct task {
    int client_fd;
    struct task* next;
} task_t;

typedef struct {
    pthread_t threads[THREAD_COUNT];
    task_t* head;
    task_t* tail;
    int queue_size;              // 🔥 NEW
    pthread_mutex_t lock;
    pthread_cond_t notify;
    int stop;
} thread_pool_t;

thread_pool_t pool;

void handle_client(int client_fd)
{
    char buffer[BUFFER];

    while(1) {
        int n = recv(client_fd, buffer, BUFFER - 1, 0);
        if (n <= 0)
            break;

        buffer[n] = '\0';
        printf("[Worker] Client %d: %s\n", client_fd, buffer);

        char* msg = "Message received\n";
        send(client_fd, msg, strlen(msg), 0);
    }

    printf("[Worker] Closing client %d\n", client_fd);
    close(client_fd);
}

void* worker_thread(void* arg)
{
    while(1)
    {
        pthread_mutex_lock(&pool.lock);

        while(pool.head == NULL && !pool.stop)
            pthread_cond_wait(&pool.notify, &pool.lock);

        if(pool.stop) {
            pthread_mutex_unlock(&pool.lock);
            break;
        }

        // 🔥 DEQUEUE
        task_t* task = pool.head;
        pool.head = task->next;

        if(pool.head == NULL)
            pool.tail = NULL;

        pool.queue_size--;   // 🔥 decrease queue size

        printf("[Worker] Picked task. Queue size: %d\n", pool.queue_size);

        pthread_mutex_unlock(&pool.lock);

        handle_client(task->client_fd);
        free(task);
    }

    return NULL;
}

void thread_pool_init()
{
    pool.head = pool.tail = NULL;
    pool.queue_size = 0;
    pool.stop = 0;

    pthread_mutex_init(&pool.lock, NULL);
    pthread_cond_init(&pool.notify, NULL);

    for(int i = 0; i < THREAD_COUNT; i++)
        pthread_create(&pool.threads[i], NULL, worker_thread, NULL);
}

void thread_pool_add_task(int client_fd)
{
    task_t* task = malloc(sizeof(task_t));
    task->client_fd = client_fd;
    task->next = NULL;

    pthread_mutex_lock(&pool.lock);

    // 🔥 BACKPRESSURE LOGIC
    if(pool.queue_size >= MAX_QUEUE) {
        printf("[Server] Queue FULL! Rejecting client %d\n", client_fd);
        pthread_mutex_unlock(&pool.lock);

        close(client_fd);
        free(task);
        return;
    }

    // 🔥 ENQUEUE
    if(pool.tail == NULL) {
        pool.head = pool.tail = task;
    } else {
        pool.tail->next = task;
        pool.tail = task;
    }

    pool.queue_size++;

    printf("[Server] Task added. Queue size: %d\n", pool.queue_size);

    pthread_cond_signal(&pool.notify);
    pthread_mutex_unlock(&pool.lock);
}

int main()
{
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if(listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("[Server] Running on port %d\n", PORT);

    thread_pool_init();

    while(1)
    {
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if(client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("[Server] New client connected: %d\n", client_fd);

        thread_pool_add_task(client_fd);
    }

    close(server_fd);
    return 0;
}
