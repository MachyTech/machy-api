#include "include/machyapi.h"

int main(int argc, char* argv[]){
    if(argc<3){
        fprintf(stderr, "usage: tcp_client hostname port\n");
        exit(-1);
    }
    
    while(1)
    {
        init_socket(argv[1], argv[2]);
        create();
        make_connection();
        run_cli();
        cleanup();
    }
}