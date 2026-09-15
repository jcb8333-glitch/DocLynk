#pragma once

#include "node.hpp"
#include "../net/netstat.hpp"
#include <queue>

class BootNode : public Node {
    private:
        NetStat net;

    public:
        BootNode(const char* selfAddr) : Node(selfAddr, selfAddr){}
};