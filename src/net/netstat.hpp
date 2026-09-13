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

        NetStat() : running_(false){}

        ~NetStat(){
            running_ = false;
        }

        void start(){
            monitorThread = std::thread(&NetStat::monitorLoop, this);
        }

        void onDiscover(nInf node){
            pendingConns.push(node);
            std::cout << "Node discovered on network, ID: " << node.id << std::endl;
        }

        void onLeave(nInf node){
            connCount--;
            std::cout << node.id << " disconnected." << std::endl;
        }

    private:
        //std::map<uint64_t, NodeStatus> netMap;
        std::mutex mapMutex;
        std::thread monitorThread;
        std::atomic<bool> running_;

        void monitorLoop(){
            while(running_){
                std::this_thread::sleep_for(std::chrono::seconds(5));
                auto now = std::chrono::steady_clock::now();
                std::lock_guard<std::mutex> lock(mapMutex);
            }
        }
        
        nInf findAuthSuccessor(uint64_t id){
            auto hit = netMap.upper_bound(id);
            if (hit == netMap.end()){
                return netMap.begin()->second;
            }
            return hit->second;
        }



};