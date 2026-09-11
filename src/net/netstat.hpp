#pragma once

#include <ostream>
#include <map>
#include <queue>
#include "../net/proto.hpp"

class NetStat {
    public:
        int connCount;
        std::map<uint64_t, nInf> netMap;
        std::queue<nInf> pendingConns;

        NetStat(){}

        ~NetStat(){}

        void activate(){
            while (true)
            {
                
            }
            
        }

        void onDiscover(nInf node){
            pendingConns.push(node);
            std::cout << "Node discovered on network, ID: " << node.id << std::endl;
        }

        void onLeave(nInf node){
            connCount--;
            std::cout << node.id << " disconnected." << std::endl;
        }

        void InitRtTables(nInf node){
            for(int i = 0; i < 64; ++i){
                uint64_t start = node.id + (1ULL << i);
                nInf owner = findAuthSuccessor(start);
                node.routeTable[i] = {start, owner};
            }
        }

    private:
        
        nInf findAuthSuccessor(uint64_t id){
            auto hit = netMap.upper_bound(id);
            if (hit == netMap.end()){
                return netMap.begin()->second;
            }
            return hit->second;
        }



};