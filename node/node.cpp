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

            struct sockaddr_in socketAddress = {
                .sin_family  = AF_INET, // Address family
                .sin_port = htons(8570), // Port number
                .sin_addr.s_addr = htonl(INADDR_ANY) // Socket address (0.0.0.0, Any IPv4 address can connect)
            };
            
        }
    public:
        Node(){}

};