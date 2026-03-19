#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 9000
#define BUFFER 1024

int total_clients =0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg){
    int client_fd = *(int*)arg;
    free(arg);
    char buffer[BUFFER];
    while(1){
    int n = recv(client_fd, buffer, BUFFER-1,0);
    if (n<=0)
        break;
    buffer[n] = '\0';
    printf("client %d --> %s\n", client_fd, buffer);
    //mutex lock to prevent race condition
    pthread_mutex_lock(&lock);
    total_clients++;
    printf("total_clients--> %d\n", total_clients);
    pthread_mutex_unlock(&lock);
    char* msg = "hi man\n";
    send(client_fd, msg, strlen(msg), 0);
    }
    close(client_fd);
    return NULL;
}

int send_all(int client_fd, void* buffer, int length){
    int total = 0;
    int n;
    char* msg = (char*)buffer;
    while (total<length){
        n = send(client_fd, msg+total, length-total,0);
        if (n<=0)
            return -1;
        total+=n;
    }
    return total;
}


int recv_all(int client_fd, void* buffer, int length){
    int total = 0;
    int n;
    char* msg = (char*)buffer;
    while (total<length){
        n = recv(client_fd, msg+total, length-total, 0);
        if (n<=0)
            return -1;
        total+=n;
    }
    return total;
}



int main(){
    //socket create server side
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM,0);
    //server and client address and client addr len
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //bind server socket to server addr
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("binding failed\n");
        exit(1);
    }
    if (listen(server_fd, 10)<0){
        printf("listening failed\n");
        exit(1);
    }
    printf("server listening on port %d\n", PORT);
    while(1){
        int* client_fd = malloc(sizeof(int));
        pthread_t tid;
        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (*client_fd<0){
            printf("accept failed\n");
            free(client_fd);
            continue;
        }
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);

    }
    close(server_fd);
    return 0;
}

