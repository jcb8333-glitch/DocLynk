#pragma once

#include <ostream>
#include "../net/proto.hpp"

class NetStat {
    public:
        static int connCount;
        std::vector<nInf> nodes;

        NetStat(){}

        ~NetStat(){}

        static void onJoin(nInf node){
            connCount++;
            std::cout << "Node discovered on network, ID: " << node.id << std::endl;
        }

        static void onDisconnect(nInf node){
            connCount--;
            std::cout << node.id << " disconnected." << std::endl;
        }

        void InitTables(Packet& packet){
            for(nInf node : nodes){
                if(node.id % connCount){
                    Packet res{MsgType::RegRes, packet.packetID, packet.chordID, node};
                }
            }
        }



};