#ifndef MACHYAPI_H
#define MACHYAPI_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

// systemcalls
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <sys/mman.h>

// socket api
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#define NO_OF_THREADS 4

#define MACHY_PORT 3333
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
/* file reader */
const char * read_file(char *file_name)
{
    int fp = open(file_name, O_RDWR);
    if (fp < 0)
    {
        fprintf(stderr, "open() failed. (%d)\n", errno);
        exit(-1);
    }

    struct stat statbuf;
    int status = fstat (fp, &statbuf);
    if(status < 0){
        fprintf(stderr, "fstat() failed. (%d)\n", errno);
        exit(-11);
    }
    
    char *file_contents = mmap (NULL, statbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fp, 0);
    if(file_contents == MAP_FAILED){
        fprintf(stderr, "mmap() failed. (%d) - %s\n", errno, strerror(errno));
        exit(-1);
    }
    close(fp);

    /* output contents */
    ssize_t n = write(1, file_contents, statbuf.st_size);
    if(n!= statbuf.st_size){
        fprintf(stderr, "write() failed. (%d)\n", errno);
    }

    return file_contents;
}

/* void unmap_file(char *ptr)
{
    if(munmap(ptr, statbuf.st_size) != 0){
        fprintf(stderr, "munmap() failed. (%d).\n", errno);
    }
} */
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

/*
buffer-overflow exploit:
Berkeley sockets by definition close upon a received message. This CLI keeps a socket open
because it assumes a default message size.
-> only use in trusted environments
-> have to change server side for this to work
*/
void run_cli_unsafe(){
    printf("To send data, enter command followed by enter.\n");
    
    while(1){

        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_connection->socket_peer, &reads);
        FD_SET(0, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;
        if (select(socket_connection->socket_peer+1, &reads, 0,0, &timeout) < 0){
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            exit(-1);
        }

        if (FD_ISSET(socket_connection->socket_peer, &reads)) {
            printf("socket open for reading\n");
            char read[16];
            int total_bytes_received = 0;
            while (total_bytes_received < 16)
            {
                int bytes_received = recv(socket_connection->socket_peer, read, 16, 0);
                if (bytes_received < 1) {
                    printf("Connection closed by peer.\n");
                    exit(-1);
                }
                total_bytes_received = total_bytes_received + bytes_received;
            }
            printf("Received (%d bytes): %.*s", total_bytes_received, total_bytes_received, read);
        }

        if (FD_ISSET(0, &reads)){
            char read[4096];
            printf("waiting...\n");
            if (!fgets(read, 4096, stdin)) break;
            printf("Sending: %s", read);
            
            read[strlen(read)-1] = '\0';
            char buf[strlen(read)+3];
            strcat(strcpy(buf, read), ":\n\n");
            
            int bytes_sent = send(socket_connection->socket_peer, buf, strlen(buf), 0);
            printf("Sent %d bytes.\n", bytes_sent);
        }
    }
}
/*
safe cli
-> does require to re-establish connection....
*/
void run_cli(){
    printf("To send data, enter command followed by enter.\n");

    while(1){
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_connection->socket_peer, &reads);
        FD_SET(0, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;
        if (select(socket_connection->socket_peer+1, &reads, 0, 0, &timeout) < 0){
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            exit(-1);
        }

        if (FD_ISSET(socket_connection->socket_peer, &reads)){
            char read[4096];
            int bytes_received = recv(socket_connection->socket_peer, read, 4096, 0);
            if(bytes_received < 1) {
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

            read[strlen(read)-1] = '\0';
            char buf[strlen(read)+3];
            strcat(strcpy(buf, read), ":\n\n");

            int bytes_sent = send(socket_connection->socket_peer, buf, strlen(buf), 0);
            printf("Sent %d bytes.\n", bytes_sent);
        }
    }
}


/* 
very basic request in a seperate thread. Recv function is problematic in this way but
good enough for "OK" response message
*/
void *run_request(void *request){
    char buf[strlen(request)+3];
    strcat(strcpy(buf, request), ":\n\n");

    int bytes_sent = send(socket_connection->socket_peer, buf, strlen(buf), 0);
    printf("Sent %d bytes.\n", bytes_sent);

    char read[4096];
    int bytes_received = recv(socket_connection->socket_peer, read, 4096, 0);
    printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);

    pthread_exit(NULL);
}

void machy_request(char *request)
{
    pthread_t tid[1];
    init_socket("0.0.0.0", "3333");

    print_addr();
    create();
    make_connection();

    pthread_create(&tid[0], NULL, run_request, request);
    //pthread_join(tid[0], NULL);
}

#endif