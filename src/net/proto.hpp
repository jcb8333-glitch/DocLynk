#pragma once

enum class MsgType : uint8_t {
    RegReq = 0,
    RegRes = 1,
    FindSuccReq = 2,
    FindSuccRes = 3,
    Ping = 4,
    Pong = 5
};

struct nInf{
    uint64_t id;
    std::string addr;
    std::string targetAddr;
    std::string secret;
    std::vector<nInf> connections;

    template <class Archive>
    void serialize(Archive& ar){
        ar(id, addr, targetAddr, secret, connections);
    }
};

struct Packet {
    MsgType type;
    uint64_t packetID;
    uint64_t chordID;
    nInf payload;

    template <class Archive>
    void serialize(Archive& ar){
        ar(type, packetID, chordID, payload);
    }
};

struct RouteEntry{
    uint64_t src;
    nInf node;
};

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