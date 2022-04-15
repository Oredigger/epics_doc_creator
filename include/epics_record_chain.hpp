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
        EpicsRecordChain(q_token &q_state);

        int  load_rec_vert(q_token &q_state);
        void traverse(int row, int col);
        void find_paths(void);
        void print_adj_mat(void);
    private:
        class EpicsRecordVertex
        {
            public:
                EpicsRecordVertex(int vert_num, 
                                  bool is_mult_links, 
                                  bool is_start_rec, 
                                  std::string name);
                
            private:
                int  vert_num;
                bool is_mult_links;
                bool is_start_rec;
                std::string name;
        };
        
        std::map<std::string, int> rec_vert;
        std::map<std::string, std::queue<std::string>> rec_links;
        std::vector<std::vector<int>> adj_mat;
};

#endif