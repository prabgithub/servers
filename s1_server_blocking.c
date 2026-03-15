#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9000
#define BUFFER 1024

int main(){

    // define socket
    int server_fd, client_fd;

    // define address
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER];
    int n;

    // inititate socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd<0){
        printf("socket creation failed\n");
        exit(1);
    }

    printf("socket created FD :%d\n", server_fd);

    // initiate address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // define bind;
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        perror("binding failed");
        exit(1);
    }

    // define listen
    if (listen(server_fd, 10)<0){
        perror("listening failed");
        exit(1);
    }

    printf("listening at port %d\n", PORT);

    // define accept

    while(1){
        printf("waiting for conncection ...\n");

        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        printf("client connected. FD %d\n", client_fd);

        while(1){
        // define handle client 
        n = recv(client_fd, buffer, BUFFER-1, 0);
        if (n<=0){
            printf("connection closed or error\n");
            close(client_fd);
            break;
        }
        buffer[n] = '\0';
        // parse message
        printf("message from FD: %d: %s\n", client_fd, buffer);


        // send response
        char *response = "message received\n";
        send(client_fd, response, strlen(response), 0);

        }


        // close clientfd
        close(client_fd);
    }
    return 0;
}

     

