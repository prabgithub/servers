#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//define macros
#define PORT 9000
#define BUFFER 1024

void main(){
    //socket
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    //address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    //convert ip addr to network representation
    inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr);
    //connect socket to serverver
    connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    //initiate buffer
    char buffer[BUFFER];

    while(1){
        fgets(buffer, BUFFER, stdin);
        send(socket_fd, buffer, strlen(buffer), 0);
        int n  = recv(socket_fd, buffer, BUFFER-1, 0);
        if (n<=0)
            break;
        //buffer[n] = "\0";
        printf("server response: %s",buffer);
    }
    close(socket_fd);

}


//connect
//send keyboard inputs
//receive response
