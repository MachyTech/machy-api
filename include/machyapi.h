#ifndef MACHYAPI_H
#define MACHYAPI_H

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

// socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

// add to heap
struct socket {
    SOCKET socket_peer;
    struct addrinfo *peer_address;
};

struct socket *socket_connection;

/*
configure a remote address for connection
*/
void init_socket(const char *ip, const char *port){
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    socket_connection = (struct socket *) malloc(sizeof(struct socket));
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(ip, port, &hints, &socket_connection->peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        exit(-1);
    } 
}

/*
It is good practice to print out the address that the program is connected to
*/
void print_addr(void){
    printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(socket_connection->peer_address->ai_addr, socket_connection->peer_address->ai_addrlen, 
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer), 
                NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);
}

/*
This function will create the socket
*/
void create(void){
    printf("Creating socket...\n");
    socket_connection->socket_peer = socket(socket_connection->peer_address->ai_family, 
        socket_connection->peer_address->ai_socktype, socket_connection->peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_connection->socket_peer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        exit(-1);
    }
}

/*
After the socket has been created, we call connect() to establish a connection to the remote server
*/

void make_connection(void){
    printf("Connecting...\n");
    if (connect(socket_connection->socket_peer, socket_connection->peer_address->ai_addr, socket_connection->peer_address->ai_addrlen)){
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        exit(-1);
    }
    freeaddrinfo(socket_connection->peer_address);
    printf("Connected.\n");
}

/* Waiting for established connection */
void wait_for_connection(void){
    while(connect(socket_connection->socket_peer, socket_connection->peer_address->ai_addr, socket_connection->peer_address->ai_addrlen)){
        printf("Connecting.. \n");
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        sleep(1);
    }
    freeaddrinfo(socket_connection->peer_address);
    printf("Connected.\n");
}

/*
some clean up
*/
void cleanup(void){
    printf("Closing socket...\n");
    CLOSESOCKET(socket_connection->socket_peer);
    free(socket_connection);
    printf("Finished.\n");
}

/*
blocking I/O
*/
void run(char *request){
    char buf[strlen(request)+3];
    strcat(strcpy(buf, request), ":\n\n");

    int bytes_sent = send(socket_connection->socket_peer, buf, strlen(buf), 0);
    printf("Sent %d bytes.\n", bytes_sent);

    char read[4096];
    int bytes_received = recv(socket_connection->socket_peer, read, 4096, 0);
    printf("Received (%d bytes): %.*s\n", bytes_received, bytes_received, read);
    cleanup();
}

/*
non-blocking
*/
void run_process(char *request){
    int pid = fork();
    if (pid == 0){
        // child process
        char buf[strlen(request)+3];
        strcat(strcpy(buf, request), ":\n\n");

        int bytes_sent = send(socket_connection->socket_peer, buf, strlen(buf), 0);
        printf("Sent %d bytes.\n", bytes_sent);

        char read[4096];
        int bytes_received = recv(socket_connection->socket_peer, read, 4096, 0);
        printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);
        exit(1);
    }
    //parent process
    cleanup();
}

void run_cli(){
    printf("To send data, enter command followed by enter.\n");
    while(1){
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_connection->socket_peer, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;
        if (select(socket_connection->socket_peer, &reads, 0,0, &timeout) < 0){
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            exit(-1);
        }
        if (FD_ISSET(socket_connection->socket_peer, &reads)) {
            char read[4096];
            int bytes_received = recv(socket_connection->socket_peer, read, 4096, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);
        }
        if (FD_ISSET(0, &reads)){
            char read[4096];
            printf("waiting...\n");
            if (!fgets(read, 4096, stdin)) break;
            printf("Sending: %s", read);
            int bytes_sent = send(socket_connection->socket_peer, read, strlen(read), 0);
            printf("Sent %d bytes.\n", bytes_sent);
        }
    }
}

#endif