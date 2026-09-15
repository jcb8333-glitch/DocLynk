#pragma once

#include <iostream>
#include <map>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include "../net/proto.hpp"

class NetStat {
    public:
        int connCount = 0;
        std::map<uint64_t, nInf> netMap;
        std::queue<nInf> pendingConns;

        NetStat() : running_(false){}

        ~NetStat(){
            running_ = false;
            if (monitorThread.joinable()) monitorThread.join();
        }

        void start(){
            running_ = true;
            monitorThread = std::thread(&NetStat::monitorLoop, this);
        }

        void onDiscover(nInf node){
            {
                std::lock_guard<std::mutex> lock(mapMutex);
                pendingConns.push(node);
            }
            std::cout << "[NetStat] Node discovered, ID: " << node.id
                       << " addr: " << node.addr << std::endl;
        }

        void onLeave(nInf node){
            connCount--;
            std::cout << "[NetStat] Node left, ID: " << node.id
                       << " (connCount now " << connCount << ")" << std::endl;
        }

    private:
        std::mutex mapMutex;
        std::thread monitorThread;
        std::atomic<bool> running_;

        void monitorLoop(){
            while (running_){
                std::this_thread::sleep_for(std::chrono::seconds(5));

                std::lock_guard<std::mutex> lock(mapMutex);
                std::cout << "[NetStat] --- status tick, pending: "
                           << pendingConns.size()
                           << ", tracked: " << netMap.size() << " ---" << std::endl;
                for (const auto& [id, info] : netMap){
                    std::cout << "  node " << id << " @ " << info.addr << std::endl;
                }
            }
        }
};