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