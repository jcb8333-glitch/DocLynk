#pragma once
// Posix socket programming
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
// Serialization
#include <cereal/archives/binary.hpp>
#include <fstream>
// General imports
#include <string>
#include <vector>
#include "config.h"

// Store data on adjacent nodes in network
struct neighbor {
    Node target;
    uint32_t weight;

    template <class Archive>
    void serialize(Archive& ar){
        ar(target, weight);
    }
};

// Contains network logic and data on a node
class Node{
    private:
        std::thread servThread;
        std::thread cliThread;

        // Server function to be executed by thread to accept connections
        int serv_sock(){
            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(sockfd < 0){
                perror("Failed to create server socket");
                return EXIT_FAILURE;
            }

            struct sockaddr_in socketAddress;
            socketAddress.sin_family  = AF_INET; // Address family
            socketAddress.sin_port = htons(8570); // Port number
            socketAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Socket address (0.0.0.0, Any IPv4 address can connect)

            if(bind(sockfd, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) < 0){
                perror("Failed to bind server socket to address");
                close(sockfd);
                return EXIT_FAILURE;
            }

            if(listen(sockfd, 16) < 0){
                perror("Server socket failed to listen");
                close(sockfd);
                return EXIT_FAILURE;
            }

            while (true){
                int connfd = accept(sockfd, NULL, NULL);
                if (connfd == -1){
                    perror("Connection refused on server socket");
                    close(sockfd);
                    return EXIT_FAILURE;
                }

                // Connection logic
                std::string msg = "Hello world!\n";
                send(connfd, msg.c_str(), msg.size(), 0);

                if(shutdown(connfd, SHUT_RDWR) == -1){
                    perror("Failed to shutdown server connection");
                    close(sockfd);
                    close(connfd);
                    return EXIT_FAILURE;
                }
                close(connfd);
            }
        }

        // Client function to be executed by a thread to connect to other nodes
        int cli_sock(){
            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sockfd < 0){
                perror("Failed to create client socket");
                return EXIT_FAILURE;
            }

            struct sockaddr_in socketAddress;
            socketAddress.sin_family = AF_INET;
            socketAddress.sin_port = htons(8570);

            int res = inet_pton(AF_INET, TEST_IP, &socketAddress.sin_addr);

            if(connect(sockfd, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) < 0){
                perror("Client failed to establish connection");
                close(sockfd);
                return EXIT_FAILURE;
            }

            // Connection logic
            char buffer[1024] = {0};

            ssize_t bytes_recv = recv(sockfd, buffer, sizeof(buffer), 0);
            if(bytes_recv < 0){
                perror("Client socket failed to receive data");
                close(sockfd);
                return EXIT_FAILURE;
            } 
            else {
                buffer[bytes_recv] = '\0';
                printf(buffer);
            }

            close(sockfd);
            return EXIT_SUCCESS;
        }

        friend class cereal::access;
        template<class Archive>
        void serialize(Archive& ar){
            ar(addr_, connections);
        }


        std::string getSelfAddr(){}

    public:

        std::string addr_ = TEST_IP;    
        std::vector<neighbor> connections;

        // Constructor: Start server and client threads on construction
        Node(){
            servThread = std::thread(&Node::serv_sock, this);
            cliThread = std::thread(&Node::cli_sock, this);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // End execution of both threads
        void joinAll(){
            if (servThread.joinable()) servThread.join();
            if (cliThread.joinable()) cliThread.join();
        }

        // Destructor: Ends threads when node is destructed
        ~Node(){
            joinAll();
        }
};