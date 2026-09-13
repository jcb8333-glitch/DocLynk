#pragma once

#include "node.hpp"
#include "../net/netstat.hpp"
#include <mutex>
#include <queue>

#include <iostream>
// Bootnode for initial node connection
class BootNode : public Node {
    private:
        NetStat net;

        void handleConnection(int connfd, Packet& packet) override {}

        void route(){}

    public:
        BootNode(const char* selfAddr) : Node(selfAddr, selfAddr){}
};