#pragma once

#include "node.hpp"

class BootNode : public Node {
    private:
        std::vector<Node> nodes;

        uint32_t calculateWeight(uint64_t ifBps, uint64_t rfBps = 100000000ULL){
            if (ifBps == 0)return 65535;
            uint64_t weight = rfBps/ifBps;
            if (weight  < 1) return 1;
            if (weight > 65535) return 65535;
            return static_cast<uint32_t>(weight);
        }

    public:
        BootNode() : Node(){}
};