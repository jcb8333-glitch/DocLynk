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
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <sstream>
// General imports
#include <string>
#include <vector>
#include "config.h"

// Debugging
#include <iostream>

// Store data on adjacent nodes in network
struct neighbor {
    std::string target_addr;
    uint32_t weight;

    template <class Archive>
    void serialize(Archive& ar){
        ar(target_addr, weight);
    }
};
struct nInf{
            std::string addr;
            std::string secret;
            std::vector<neighbor> connections;

            template <class Archive>
            void serialize(Archive& ar){
                ar(addr, secret, connections);
            }
        };

// Contains network logic and data on a node
class Node{
    private:
        std::string addr_ = TEST_IP;
        std::thread servThread;
        std::thread cliThread;
        std::string secret_ = "Im trapped in a for loop";  
        std::vector<neighbor> connections_;
        // Struct for serialization
        struct nInf nodeInfo;

        // Send data on self over connection for network discovery
        int sendNode(int sockfd, nInf& node){
            std::stringstream ss;
            {
                cereal::BinaryOutputArchive archive(ss);
                archive(node);
            }
            std::string payload = ss.str();
            uint32_t len = htonl(static_cast<uint32_t>(payload.size()));
            if (send(sockfd, &len, sizeof(len), 0) != sizeof(len)) return -1;
            if (send(sockfd, payload.data(), payload.size(), 0) != (ssize_t)payload.size()) return -2;
            return 0;
        }
        // Receive serialized node
        int recvNode(int sockfd, nInf& node){
            uint32_t len;
            if(recv(sockfd, &len, sizeof(len), MSG_WAITALL) != sizeof(len)) return -1;
            len = ntohl(len);
            std::string payload(len, '\0');
            if (recv(sockfd, payload.data(), len, MSG_WAITALL) != (ssize_t)len) return -2;
            std::stringstream ss(payload);
            cereal::BinaryInputArchive archive(ss);
            archive(node);
            return 0;
        }

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
                nInf incoming;
                if(recvNode(connfd, incoming) < 0){
                    perror("Server thread failed to receive serialized node");
                    return EXIT_FAILURE;
                } else {
                    std::cout << incoming.secret << std::endl;
                }


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

            if(sendNode(sockfd, nodeInfo) < 0){
                perror("Client thread failed to serialize node");
                close(sockfd);
                return EXIT_FAILURE;
            }

            close(sockfd);
            return EXIT_SUCCESS;
        }

        // std::string getSelfAddr(){}

    public:

        // Constructor: Start server and client threads on construction
        Node(){
            nodeInfo.addr = addr_;
            nodeInfo.secret = secret_;
            nodeInfo.connections = connections_;
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

struct neighbors {
    Node target;
    uint32_t weight;
};