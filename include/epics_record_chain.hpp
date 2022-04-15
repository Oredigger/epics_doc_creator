#ifndef EPICS_RECORD_CHAIN_H
#define EPICS_RECORD_CHAIN_H

#include <map>
#include <vector>
#include "epics_lex_analysis.hpp"

class EpicsRecordVertex
{
    public:
        EpicsRecordVertex(void);
    private:
        int  vert_num;
        bool is_mult_links;
        bool is_start_rec;
};

class EpicsRecordChain
{
    public:
        EpicsRecordChain(void);
        EpicsRecordChain(q_token &q_state);

        size_t load_rec_vert(q_token &q_state);
        void find_paths(void);
        void print_adj_mat(void);
    private:
        std::map<std::string, int> rec_vert;
        std::map<std::string, std::queue<std::string>> rec_links;
        std::vector<std::vector<int>> adj_mat;
};

#endif