#include <cstdio>
#include <cstdlib>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class Node{
    private:
        int lconn(){
            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(sockfd == -1){
                fprintf(stderr, "Failed to create socket\n");
                return EXIT_FAILURE;
            }

            struct sockaddr_in socketAddress = {
                .sin_family  = AF_INET, // Address family
                .sin_port = htons(8570), // Port number
                .sin_addr.s_addr = htonl(INADDR_ANY) // Socket address (0.0.0.0, Any IPv4 address can connect)
            };

            if(bind(sockfd, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) == -1){
                fprintf(stderr, "Failed to bind socket to address\n");
                close(sockfd);
                return EXIT_FAILURE;
            }

            if(listen(sockfd, 16) == -1){
                fprintf(stderr, "Socket failed to listen\n");
                close(sockfd);
                return EXIT_FAILURE;
            }

            while (true){
                int connfd = accept(sockfd, NULL, NULL);
                if (connfd == -1){
                    fprintf(stderr, "Connection refused\n");
                    close(sockfd);
                    return EXIT_FAILURE;
                }

                // Connection logic

                if(shutdown(connfd, SHUT_RDWR) == -1){
                    fprintf(stderr, "Failed to shutdown connection\n");
                    close(sockfd);
                    close(connfd);
                    return EXIT_FAILURE;
                }
                close(connfd);
            }
            close(sockfd);
        }
    public:
        Node(){}

};