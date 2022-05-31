#ifndef EPICS_RECORD_CHAIN_H
#define EPICS_RECORD_CHAIN_H

#include <map>
#include <string>
#include <vector>
#include "epics_lex_analysis.hpp"

class EpicsRecordChain
{
    public:
        EpicsRecordChain(void);
        EpicsRecordChain(q_token q_state);

        int  load_rec_vert(q_token q_state);
        bool is_start_chain(std::string vert_name);
        void create_visual_graph(std::string fn);
        void print_adj_mat(void);
    private:
        int n;
        std::map<std::string, int> rec_vert;
        std::map<std::string, std::queue<std::string>> rec_links;
        std::vector<std::vector<int>> adj_mat;
};

#endif