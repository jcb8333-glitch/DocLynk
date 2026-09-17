#pragma once

#include "node.hpp"
#include <queue>

class BootNode : public Node {
    public:
        BootNode(const char* selfAddr) : Node(selfAddr, selfAddr){}
};