#pragma once
// Posix socket programming
#include <cstdio>
#include <cstdlib>
#include <future>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
// Serialization
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
// Hashing
#include "../hash/sha1.hpp"

#include <sstream>
// General imports
#include <string>
#include <cstring>
#include <vector>
// Debugging
#include <iostream>
#include "../net/proto.hpp"

class PeerConn {
    public:
        int sockfd;
        std::string peerAddr;
        std::thread reader;
        std::atomic<bool> alive{true};
        std::mutex sendMutex;
        std::mutex pendingMutex;
        std::unordered_map<uint64_t, std::promise<Packet>> pending; 

        PeerConn(int fd, std::string addr) : sockfd(fd), peerAddr(std::move(addr)){}

        ~PeerConn(){
            alive = false;
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            if (reader.joinable()) reader.join();
        }
};

// Contains network logic and data on a node
class Node{
    private:
        const uint64_t id_;
        const char* addr_;
        const char* targetAddr_;
        std::promise<void> sReady_;
        std::shared_future<void> sReadyFuture_;
        std::thread servThread;
        std::thread cliThread;
        std::string secret_ = "Im trapped in a for loop";
        nInf successor_;
        nInf predecessor_;
        std::vector<RouteEntry> routeTable_;
        std::vector<nInf> connections_;
        struct nInf nodeInfo;
        std::atomic<uint64_t> nextRequestID{1};
        std::mutex poolMutex;
        std::unordered_map<std::string, std::shared_ptr<PeerConn>> connectionPool;

        nInf remoteFindSuccessor(std::shared_ptr<PeerConn> conn, uint64_t chordID){
            uint64_t reqID = nextRequestID++;

            std::promise<Packet> respPromise;
            std::future<Packet> respFuture = respPromise.get_future();
            {
                std::lock_guard<std::mutex> lock(conn->pendingMutex);
                conn->pending[reqID] = std::move(respPromise);
            }

            Packet req{MsgType::FindSuccReq, reqID, chordID, nodeInfo};
            {
                std::lock_guard<std::mutex> lock(conn->sendMutex);
                sendPacket(conn->sockfd, req);
            }

            auto status = respFuture.wait_for(std::chrono::seconds(5));
            if(status != std::future_status::ready){
                return nInf{};
            }
            return respFuture.get().payload;
        }

        void updateNodeInfo(nInf node){
            // Currently only updating routing table to configure network
            // Add more vars as needed
        }

        // Loop to read incoming packets
        void readLoop(std::shared_ptr<PeerConn> conn){
            while (conn->alive){
                Packet packet;
                if (recvPacket(conn->sockfd, packet) < 0){
                    conn->alive = false;
                    break;
                }

                switch (packet.type){
                    case MsgType::RtReq:
                        break;
                    case MsgType::RtRes:
                        successor_=packet.payload;
                        updateNodeInfo(successor_);
                        getOrConnect(successor_.addr);
                        break;
                    default:
                        break;
                }

                if(packet.type == MsgType::FindSuccRes || packet.type == MsgType::Pong){
                    std::lock_guard<std::mutex> lock(conn->pendingMutex);
                    auto it = conn->pending.find(packet.packetID);
                    if(it != conn->pending.end()){
                        it->second.set_value(packet);
                        conn->pending.erase(it);
                    }
                } else {
                    handleConnection(conn->sockfd, packet);
                }
            }
        }

        bool inRange(uint64_t id, uint64_t src, uint64_t dst, bool inclusive=false){
            if (src < dst){
                return inclusive ? (id > src && id <= dst) : (id > src && id < dst);
            } else {
                return inclusive ? (id > src || id <= dst) : (id > src || id < dst);
            }
        }

        nInf closestPrecedingNode(uint64_t id){
            for (int i = 63; i >= 0; --i){
                if(routeTable_[i].node.id != 0 && inRange(routeTable_[i].node.id, id_, id)){
                    return routeTable_[i].node;
                }
            }
            nInf self;
            self.id = id_;
            self.addr = addr_;
            return self;
        }

        std::shared_ptr<PeerConn> getOrConnect(const std::string& peerAddr){
            {
                std::lock_guard<std::mutex> lock(poolMutex);
                auto it = connectionPool.find(peerAddr);
                if (it != connectionPool.end() && it->second->alive) return it->second;
            }

            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(sockfd < 0) return nullptr;

            struct sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(8570);
            inet_pton(AF_INET, peerAddr.c_str(), &addr.sin_addr);

            if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
                close(sockfd);
                return nullptr;
            }

            auto conn = std::make_shared<PeerConn>(sockfd, peerAddr);
            conn->reader = std::thread(&Node::readLoop, this, conn);

            std::lock_guard<std::mutex> lock(poolMutex);
            connectionPool[peerAddr] = conn;
            return conn;
        }

        // Server function to be executed by thread to accept connections
        int serv_sock(){
            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(sockfd < 0){
                perror("Failed to create server socket");
                sReady_.set_value();
                return EXIT_FAILURE;
            }

            struct sockaddr_in socketAddress;
            socketAddress.sin_family  = AF_INET; // Address family
            socketAddress.sin_port = htons(8570); // Port number
            socketAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Socket address (0.0.0.0, Any IPv4 address can connect)

            if(bind(sockfd, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) < 0){
                perror("Failed to bind server socket to address");
                close(sockfd);
                sReady_.set_value();
                return EXIT_FAILURE;
            }

            if(listen(sockfd, 16) < 0){
                perror("Server socket failed to listen");
                close(sockfd);
                sReady_.set_value();
                return EXIT_FAILURE;
            }

            sReady_.set_value();

            while (true){
                int connfd = accept(sockfd, NULL, NULL);
                if (connfd == -1){
                    perror("Connection refused on server socket");
                    continue;
                }

                auto conn = std::make_shared<PeerConn>(connfd, "N/A");
                conn->reader = std::thread(&Node::readLoop, this, conn);
                conn->reader.detach();
            }
        }

        // Client function to be executed by a thread to connect to other nodes
        int cli_sock(){
            if (std::strcmp(targetAddr_, addr_) == 0){
                return EXIT_FAILURE;
            }
            sReadyFuture_.wait();
            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sockfd < 0){
                perror("Failed to create client socket");
                return EXIT_FAILURE;
            }

            struct sockaddr_in socketAddress;
            socketAddress.sin_family = AF_INET;
            socketAddress.sin_port = htons(8570);

            int res = inet_pton(AF_INET, targetAddr_, &socketAddress.sin_addr);

            if(connect(sockfd, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) < 0){
                perror("Client failed to establish connection");
                close(sockfd);
                return EXIT_FAILURE;
            }

            // Connection logic
            char buffer[1024] = {0};
            Packet pack{MsgType::RtReq, nextRequestID++, id_, nodeInfo};

            if(sendPacket(sockfd, pack) < 0){
                perror("Client thread failed to serialize node");
                close(sockfd);
                return EXIT_FAILURE;
            }

            close(sockfd);
            return EXIT_SUCCESS;
        }

    public:

        // Constructor: Start server and client threads on construction
        Node(const char* selfAddr, const char* bootAddr)
            : addr_(selfAddr), targetAddr_(bootAddr), id_(sha1Trunc(addr_))
        {
            nodeInfo.id = id_;
            nodeInfo.addr = addr_;
            nodeInfo.targetAddr = targetAddr_;
            nodeInfo.routeTable = routeTable_;
            nodeInfo.connections = {};

            sReadyFuture_ = sReady_.get_future();
            servThread = std::thread(&Node::serv_sock, this);
            cliThread = std::thread(&Node::cli_sock, this);
        }

        // End execution of both threads
        void joinAll(){
            if (servThread.joinable()) servThread.join();
            if (cliThread.joinable()) cliThread.join();
        }

        // Destructor: Ends threads when node is destructed
        virtual ~Node(){
            joinAll();
        }

        nInf findSuccessor(uint64_t id){
            if(inRange(id, id_, successor_.id, true)){
                return successor_;
            } else {
                nInf n0 = closestPrecedingNode(id);
                auto conn = getOrConnect(n0.addr);
                if(!conn) return n0;
                return remoteFindSuccessor(conn, id);
            }
        }

        virtual void handleConnection(int sockfd, Packet& packet){
            if (packet.type == MsgType::FindSuccReq) {
                nInf res = findSuccessor(packet.chordID);
                Packet resp{MsgType::FindSuccRes, packet.packetID, packet.chordID, res};
                sendPacket(sockfd, resp);
            }
        }
};