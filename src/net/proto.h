#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Stores values for the type of message in a packet.
enum class MsgType : uint8_t {
    GetPredReq = 0, GetPredRes = 1,
    NotifyReq = 2, NotifyRes = 3,
    FindSuccReq = 6, FindSuccRes = 7,
    Ping = 4, Pong = 5
};

// Stores data in a node's routing table.
// ID of the starting node, ID of the destination, the address of the destination.
struct RouteEntry{
    uint64_t startID;
    uint64_t nodeID;
    std::string addr;

    template <class Archive>
    void serialize(Archive& ar){
        ar(startID, nodeID, addr);
    }
};

// Struct to hold node data to be serialized and sent over a connection.
// Prevents threads and functions from being sent
struct nInf{
    uint64_t id;
    std::string addr;
    std::string targetAddr;
    std::vector<RouteEntry> routeTable;
    std::vector<nInf> connections;

    template <class Archive>
    void serialize(Archive& ar){
        ar(id, addr, targetAddr, routeTable, connections);
    }
};

constexpr uint32_t kMaxPacket = 1 << 20;

// Packet inserted into the payload of a TCP segment.
// Contains message type for correct handling, an ID to connect a response packet to a request,
// the chord ID of the destination node, and a payload containing needed node data
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

inline int sendAll(int fd, const char* p, size_t n){
    while(n){
        ssize_t w = send(fd, p, n, MSG_NOSIGNAL);
        if (w < 0 && errno == EINTR) continue;
        if (w <= 0) return -1;
        p += w; n -= w;
    }
    return 0;
}

// Serializes a packet and its contents into binary to be sent over a socket.
inline int sendPacket(int sockfd, Packet& packet){
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive archive(ss);
        archive(packet);
    }
    std::string payload = ss.str();
    uint32_t len = htonl(static_cast<uint32_t>(payload.size()));
    if (sendAll(sockfd, reinterpret_cast<const char*>(&len), sizeof(len)) != sizeof(len)) return -1;
    if (sendAll(sockfd, payload.data(), payload.size()) != (ssize_t)payload.size()) return -2;
    return 0;
}

// De-serializes a packet and its contents that was received over a socket.
inline int recvPacket(int sockfd, Packet& packet){
    uint32_t len;
    if(recv(sockfd, &len, sizeof(len), MSG_WAITALL) != sizeof(len)) return -1;
    len = ntohl(len);
    if(len == 0 || len > kMaxPacket) return -3;
    std::string payload(len, '\0');
    if (recv(sockfd, payload.data(), len, MSG_WAITALL) != (ssize_t)len) return -2;
    std::stringstream ss(payload);
    cereal::BinaryInputArchive archive(ss);
    archive(packet);
    return 0;
}

// Variant of recvPacket
// Puts available bytes into a buffer, then attempts to parse the packet
inline int recvFrame(int sockfd, std::string& rbuf, Packet& packet){
    char tmp[4096];
    for(;;){
        uint32_t len;
        std::memcpy(&len, rbuf.data(), sizeof(len));
        len = ntohl(len);
        if(len == 0 || len > kMaxPacket) return -3;

        if (rbuf.size() >= sizeof(len) + len){
            std::stringstream ss(rbuf.substr(sizeof(len), len));
            rbuf.erase(0, sizeof(len) + len);
            try {
                cereal::BinaryInputArchive archive(ss);
                archive(packet);
            } catch (...){
                return -4
            }
            return 0;
        }
    }

    ssize_t n = recv(sockfd, tmp, sizeof(tmp), MSG_DONTWAIT);
    if (n > 0){
        rbuf.append(tmp, static_cast<size_t>(n));
        continue;
    }
    if (n == 0) return -1;
    if (errno == EINTR) continue;
    if (errno == EAGIN || errno == EWOULDBLOCK) return 1;
    return -2;
}