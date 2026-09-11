#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
    std::vector<RouteEntry> routeTable;
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

int sendPacket(int sockfd, Packet& packet){
            std::stringstream ss;
            {
                cereal::BinaryOutputArchive archive(ss);
                archive(packet);
            }
            std::string payload = ss.str();
            uint32_t len = htonl(static_cast<uint32_t>(payload.size()));
            if (send(sockfd, &len, sizeof(len), 0) != sizeof(len)) return -1;
            if (send(sockfd, payload.data(), payload.size(), 0) != (ssize_t)payload.size()) return -2;
            return 0;
        }
        // Receive serialized node
        int recvPacket(int sockfd, Packet& packet){
            uint32_t len;
            if(recv(sockfd, &len, sizeof(len), MSG_WAITALL) != sizeof(len)) return -1;
            len = ntohl(len);
            std::string payload(len, '\0');
            if (recv(sockfd, payload.data(), len, MSG_WAITALL) != (ssize_t)len) return -2;
            std::stringstream ss(payload);
            cereal::BinaryInputArchive archive(ss);
            archive(packet);
            return 0;
        }