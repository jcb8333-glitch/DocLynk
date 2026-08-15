#include <cstdio>
#include <cstdlib>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class Node{
    private:
        int listen(){
            int sockL = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(sockL == -1){
                fprintf(stderr, "Failed to create socket\n");
                return EXIT_FAILURE;
            }
        }
    public:
        Node(){}

};