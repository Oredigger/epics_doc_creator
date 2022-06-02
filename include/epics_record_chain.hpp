#ifndef EPICS_RECORD_CHAIN_H
#define EPICS_RECORD_CHAIN_H

#include "epics_lex_analysis.hpp"
#include <map>
#include <string>
#include <tuple>
#include <vector>

enum Link_Type {NO_LINK = 0, FLNK = 1, OUT = 2, INP = 3};
enum Link_Prop {NO_PROP = 0, PP = 1, CA = 2, CPP = 4, CP = 8};

class EpicsRecordChain
{
    public:
        EpicsRecordChain(void);
        EpicsRecordChain(q_token q_state);

        int  load_rec_vert(q_token q_state);
        void create_visual_graph(std::string fn);
        void print_adj_mat(void);
    private:
        int n;
        std::map<std::string, int> rec_vert;
        std::map<std::string, std::string> rec_types;
        std::map<std::string, std::queue<std::tuple<std::string, Link_Type, int>>> rec_links;
        std::vector<std::vector<std::tuple<Link_Type, int>>> adj_mat;
};

#endif