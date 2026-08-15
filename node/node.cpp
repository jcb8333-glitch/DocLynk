#include <cstdio>
#include <cstdlib>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class Node{
    private:
        int serv_sock(){
            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(sockfd == -1){
                fprintf(stderr, "Failed to create server socket\n");
                return EXIT_FAILURE;
            }

            struct sockaddr_in socketAddress = {
                .sin_family  = AF_INET, // Address family
                .sin_port = htons(8570), // Port number
                .sin_addr.s_addr = htonl(INADDR_ANY) // Socket address (0.0.0.0, Any IPv4 address can connect)
            };

            if(bind(sockfd, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) == -1){
                fprintf(stderr, "Failed to bind server socket to address\n");
                close(sockfd);
                return EXIT_FAILURE;
            }

            if(listen(sockfd, 16) == -1){
                fprintf(stderr, "Server socket failed to listen\n");
                close(sockfd);
                return EXIT_FAILURE;
            }

            while (true){
                int connfd = accept(sockfd, NULL, NULL);
                if (connfd == -1){
                    fprintf(stderr, "Connection refused on server socket\n");
                    close(sockfd);
                    return EXIT_FAILURE;
                }

                // Connection logic

                if(shutdown(connfd, SHUT_RDWR) == -1){
                    fprintf(stderr, "Failed to shutdown server connection\n");
                    close(sockfd);
                    close(connfd);
                    return EXIT_FAILURE;
                }
                close(connfd);
            }
            close(sockfd);
        }

        int cli_sock(){
            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sockfd == -1){
                fprintf(stderr, "Failed to create client socket\n");
                return EXIT_FAILURE;
            }

            struct sockaddr_in socketAddress {
                .sin_family = AF_INET,
                .sin_port = htons(8570)
            };
            int res = inet_pton(AF_INET, /*TEST IP*/, &socketAddress.sin_addr);

            if(connect(sockfd, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) == -1){
                fprintf(stderr, "Client failed to establish connection\n");
                close(sockfd);
                return EXIT_FAILURE;
            }

            // Connection logic

            close(sockfd);
            return EXIT_SUCCESS;
        }
    public:
        Node(){}

};