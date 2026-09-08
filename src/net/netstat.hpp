#pragma once

#include <ostream>
#include "../node/node.hpp"

class NetStat {
    public:
        std::vector<nInf> nodes;

        NetStat(){}

        void onJoin(nInf node){
            std::cout << "Node discovered on network, ID: " << node.id << std::endl;
        }

        ~NetStat(){}

};