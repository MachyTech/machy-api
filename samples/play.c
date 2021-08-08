#include "include/machyapi.h"

int main(int argc, char* argv[]){
    if(argc<3){
        fprintf(stderr, "usage: tcp_client hostname port\n");
        exit(-1);
    }

    //init_socket(argv[1], argv[2]);

    //print_addr();
    //create();
    //wait_for_connection();

    printf("run process\n");
    //run_process("TEST000001");

    // start new

    printf("machy request\n");
    machy_request("TEST000001");
    // simulating real-time while loop...
    while(1){}

    return 1;
}
