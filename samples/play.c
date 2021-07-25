#include "include/machyapi.h"

int main(int argc, char* argv[]){
    if(argc<3){
        fprintf(stderr, "usage: tcp_client hostname port\n");
        exit(-1);
    }

    init_socket(argv[1], argv[2]);

    print_addr();
    create();
    wait_for_connection();

    printf("run process\n");
    run_process("TEST000001");

    // start new

    init_socket(argv[1], argv[2]);

    print_addr();
    create();
    wait_for_connection();

    printf("run\n");
    run("ECHO000001");
    
    return 1;
}
