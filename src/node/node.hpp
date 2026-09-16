#pragma once

// Posix & C libs
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
// Serialization
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
// Concurrency
#include <thread>
#include <future>
#include <atomic>
#include <chrono>
// Standard lib
#include <array>
#include <unordered_map>
#include <optional>
#include <ostream>

#include "../net/proto.hpp"
#include "../hash/sha1.hpp"

/*
PeerConn represents the connection between nodes.
When a connection is made in Node, a TCP connection is made to reference the connection and hold the file descriptor
and manage thread locking and promises. Destructing the PeerConn breaks the connection and closes the socket.
Creates a reader thread to be accessed by a Node to read data received on a connection.
*/
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

/*
Node manages P2P connection between clients on the network in a chord DHT configuration.
Uses server and client thread to hold a connection and uses a stabilizer thread to maintain
connections on the network. 
*/
class Node{
    public:
        // Constructor: Copies data to nInf struct and starts server, client, and stabilizer threads
        Node(const char* selfAddr, const char* bootAddr)
        : addr_(selfAddr), targetAddr_(bootAddr), id_(sha1Trunc(addr_))
        {
            std::vector<RouteEntry> rt(routeTable_.begin(), routeTable_.end());
            nodeInfo.id = id_;
            nodeInfo.addr = addr_;
            nodeInfo.targetAddr = targetAddr_;
            nodeInfo.routeTable = rt;
            nodeInfo.connections = {};

            successor_ = nodeInfo;
            successorList_.push_back(successor_);

            sReadyFuture_ = sReady_.get_future();
            servThread = std::thread(&Node::serv_sock, this);
            cliThread = std::thread(&Node::cli_sock, this);
            stabilizeThread =std::thread(&Node::stabilizeLoop, this);
        }

        // Destructor: Sets a Node to no longer be running and joins all threads 
        ~Node(){
            running_ = false;
            joinAll();
        }

        // Calls functions to check integrity and stabilize the node's network connection while the node is running
        void stabilizeLoop(){
            while(running_){
                stabilize();
                updateRtTable();
                checkPredecessor();

                nInf succ, pred;
                {
                    std::lock_guard<std::mutex> lock(succMutex_);
                    succ = successor_;
                }
                {
                    std::lock_guard<std::mutex> lock(predMutex_);
                    pred = predecessor_;
                }
                std::cout << "[Node " << id_ << "] successor: "
                    << (isUnset(succ) ? "none" : std::to_string(succ.id) + " (" + succ.addr + ")")
                    << " | predecessor: "
                    << (isUnset(pred) ? "none" : std::to_string(pred.id) + " (" + pred.addr + ")")
                    << std::endl;

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

    private:
        // Node data
        const char* addr_;
        const char* targetAddr_;
        const uint64_t id_;
        int fingerIdx_ = 0;
        struct nInf nodeInfo;
        nInf successor_;
        nInf predecessor_;
        std::vector<nInf> successorList_;
        std::array<RouteEntry, 64> routeTable_;
        std::unordered_map<std::string, std::shared_ptr<PeerConn>> connectionPool;
        // Concurrency objects
        std::promise<void> sReady_;
        std::shared_future<void> sReadyFuture_;
        std::thread servThread;
        std::thread cliThread;
        std::thread stabilizeThread;
        std::mutex succMutex_;
        std::mutex predMutex_;
        std::mutex poolMutex;
        std::atomic<bool> running_{true};
        std::atomic<uint64_t> nextRequestID{1};

        // Joins all threads if joinable. Used during Node destruction
        void joinAll(){
            if (servThread.joinable()) servThread.join();
            if (cliThread.joinable()) cliThread.join();
            if (stabilizeThread.joinable()) stabilizeThread.join();
        }

        // Searches range to return the nearest predecessor connected to a node.
        // If the predecessor is not found inside of the range the nearest predecessor
        // defaults to itself.
        nInf closestPrecedingNode(uint64_t id){
            for (int i = 63; i >= 0; --i){
                if (routeTable_[i].nodeID != 0 && inRange(routeTable_[i].nodeID, id_, id)){
                    nInf n;
                    n.id = routeTable_[i].nodeID;
                    n.addr = routeTable_[i].addr;
                    return n;
                }
            }
            nInf self;
            self.id = id_;
            self.addr = addr_;
            return self;
        }

        // Returns the successor of a node via an id if the successor is in the node's range
        // otherwise attempts to find a predecessor with a valid successor in the range.
        nInf findSuccessor(uint64_t id){
            nInf succ;
            {
                std::lock_guard<std::mutex> lock(succMutex_);
                succ = successor_;
            }
            if(inRange(id, id_, succ.id, true)){
                return succ;
            } else {
                nInf n0 = closestPrecedingNode(id);
                auto conn = getOrConnect(n0.addr);
                if(!conn) return n0;
                return remoteFindSuccessor(conn, id);
            }
        }

        // Requests a remote node to call its local variant of a function and reply with a response.
        std::optional<Packet> remoteCall(std::shared_ptr<PeerConn> conn, Packet req){
        uint64_t reqID = req.packetID;
            std::promise<Packet> resPromise;
            std::future<Packet> resFuture = resPromise.get_future();
            {
                std::lock_guard<std::mutex> lock(conn->pendingMutex);
                conn->pending[reqID] = std::move(resPromise);
            }
            {
                std::lock_guard<std::mutex> lock(conn->sendMutex);
                if(sendPacket(conn->sockfd, req) < 0){
                    std::lock_guard<std::mutex> lock2(conn->pendingMutex);
                    conn->pending.erase(reqID);
                    return std::nullopt;
                }
            }
            auto status = resFuture.wait_for(std::chrono::seconds(5));
            if(status != std::future_status::ready){
                std::lock_guard<std::mutex> lock(conn->pendingMutex);
                conn->pending.erase(reqID);
                return std::nullopt;
            }
            return resFuture.get();
        }

        /*
        RPC variants of other network functions.
        Construct a packet to get sent to another node to perform an operation
        rather than the local node performing that operation.
        */
        // Returns the successor of the a node via its chordID
        nInf remoteFindSuccessor(std::shared_ptr<PeerConn> conn, uint64_t chordID){
            Packet req{MsgType::FindSuccReq, nextRequestID++, chordID, nodeInfo};
            auto res = remoteCall(conn, req);
            return res ? res->payload : nInf{};
        }
        // Returns the predecessor belonging to the other node on the connection
        nInf remoteFindPredecessor(std::shared_ptr<PeerConn> conn){
            Packet req{MsgType::GetPredReq, nextRequestID++, 0, nodeInfo};
            auto res = remoteCall(conn, req);
            return res ? res->payload : nInf{};
        }

        // Gets passed into reader thread to read incoming packets while
        // connection is alive.
        void readLoop(std::shared_ptr<PeerConn> conn){
            while (conn->alive){
                Packet packet;
                if (recvPacket(conn->sockfd, packet) < 0){
                    conn->alive = false;
                    break;
                }

                switch (packet.type){
                    case MsgType::Ping:{
                        Packet res{MsgType::Pong, packet.packetID, packet.chordID, nodeInfo};
                        sendPacket(conn->sockfd, res);
                        break;
                    }
                    default:{
                        break;
                    }
                }
                if ( packet.type == MsgType::FindSuccRes ||  packet.type == MsgType::GetPredRes ||  packet.type == MsgType::Pong){
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

        /*
        The following series of functions handle P2P node connection and stabilization.
        P2P helpers are also included in this list.
        */
        // Checks if a node fits into the range between two nodes
        bool inRange(uint64_t id, uint64_t src, uint64_t dst, bool inclusive=false){
            if (src < dst){
                return inclusive ? (id > src && id <= dst) : (id > src && id < dst);
            } else {
                return inclusive ? (id > src || id <= dst) : (id > src || id < dst);
            }
        }

        // Checks if a node is dead by looking at the address
        bool isUnset(const nInf& node){return node.addr.empty();}

        // Changes the predecessor and successor to new nodes if either is found to be unset.
        // Creates a connection between new predecessor or successor.
        void stabilize(){
            nInf succ;
            {
                std::lock_guard<std::mutex> lock(succMutex_);
                succ = successor_;
            }
            if (isUnset(succ)) return;

            auto conn = getOrConnect(succ.addr);
            if (!conn) return;

            nInf x = remoteFindPredecessor(conn);
            if(!isUnset(x) && inRange(x.id, id_, succ.id)){
                std::lock_guard<std::mutex> lock(succMutex_);
                successor_ = x;
                succ = x;
            }

            auto succConn = getOrConnect(succ.addr);
            if (succConn){
                Packet req{MsgType::NotifyReq, nextRequestID++, 0, nodeInfo};
                std::lock_guard<std::mutex> lock(succConn->sendMutex);
                sendPacket(succConn->sockfd, req);
            }

        }

        // Notifies the network that a new node is looking for a place on the network
        void notify(nInf candidate){
            std::lock_guard<std::mutex> lock(predMutex_);
            if(isUnset(predecessor_) || inRange(candidate.id, predecessor_.id, id_)){
                predecessor_ = candidate;
            }
        }

        // Updates the finger table with up to date route entries
        void updateRtTable(){
            fingerIdx_ = (fingerIdx_ % 64) + 1;
            uint64_t start = id_ + (1ULL << (fingerIdx_ - 1));
            nInf owner = findSuccessor(start);
            routeTable_[fingerIdx_ - 1] = {start, owner.id, owner.addr};
        }

        // Checks if the predecessor is set to prevent dead nodes from remaining on the network.
        void checkPredecessor(){
            nInf pred;
            {
                std::lock_guard<std::mutex> lock(predMutex_);
                pred = predecessor_;
            }
            if (isUnset(pred)) return;

            auto conn = getOrConnect(pred.addr);
            if(!conn){
                std::lock_guard<std::mutex> lock(predMutex_);
                predecessor_ = nInf{};
                return;
            }

            Packet req{MsgType::Ping, nextRequestID++, 0, nodeInfo};
            auto res = remoteCall(conn, req);
            if(!res){
                std::lock_guard<std::mutex> lock(predMutex_);
                predecessor_ = nInf{};
            }
        }

        // Filters request packets to perfom proper operation and send back with a response packet
        virtual void handleConnection(int sockfd, Packet& packet){
            switch (packet.type){
                case MsgType::FindSuccReq: {
                    nInf res = findSuccessor(packet.chordID);
                    Packet resp{MsgType::FindSuccRes, packet.packetID, packet.chordID, res};
                    sendPacket(sockfd, resp);
                    break;
                }
                case MsgType::GetPredReq: {
                    nInf pred;
                    {
                        std::lock_guard<std::mutex> lock(predMutex_);
                        pred = predecessor_;
                    }
                    Packet resp{MsgType::GetPredRes, packet.packetID, packet.chordID, pred};
                    sendPacket(sockfd, resp);
                    break;
                }
                case MsgType::NotifyReq:
                    notify(packet.payload);
                    break;
                default:
                    break;
            }
        }

        // Establishes a connection to a node with a specifed address if
        // a connection can't be found in a node's connection pool with associated peer address,
        // otherwise return connection associated with the peer address.
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

        // Contains logic for the server thread on a node.
        // Creates a socket listening for incoming TCP connections.
        // On connection a new socket is made with the closed connection
        // and a reader thread is created to read on this end of the connection
        int serv_sock(){
            int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(sockfd < 0){
                perror("Failed to create server socket");
                sReady_.set_value();
                return EXIT_FAILURE;
            }

            struct sockaddr_in socketAddress;
            socketAddress.sin_family  = AF_INET;
            socketAddress.sin_port = htons(8570);
            socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

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

        // Contains logic for the client thread on a node.
        // Calls getOrConnect to attempt to create a connection to the node linked to
        // targetAddr_. 
        int cli_sock(){
            if (std::strcmp(targetAddr_, addr_) == 0){
                return EXIT_FAILURE;
            }
            sReadyFuture_.wait();

            auto conn = getOrConnect(targetAddr_);
            if (!conn){
                perror("Client failed to establish connection to node");
                return EXIT_FAILURE;
            }

            nInf succ = remoteFindSuccessor(conn, id_);
            if(isUnset(succ)){
                perror("Join failed: Node did not return a valid successor");
                return EXIT_FAILURE;
            }
            {
                std::lock_guard<std::mutex> lock(succMutex_);
                successor_ = succ;
            }
            return EXIT_SUCCESS;
        }
};