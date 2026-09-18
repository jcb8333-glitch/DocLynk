#pragma once

#include <node.h>
#include <queue>

class BootNode : public Node {
    public:
        BootNode(const char* selfAddr) : Node(selfAddr, selfAddr){}
};