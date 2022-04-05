#ifndef EPICS_LATEX_GEN_H
#define EPICS_LATEX_GEN_H

#include <string>
#include "epics_lex_analysis.hpp"

class EpicsLatexHeader
{
    public:
        EpicsLatexHeader(std::string dbh_str);
        std::string get_conv_str(void);

    private:
        std::string conv;
        std::string brief;
        std::string bug;
        std::string desc;
        std::string file;
};

std::string gen_header_str(std::string db_fn, q_token &q_state);

void gen_latex_doc(std::string tex_fn, std::string db_fn, q_token q_state);

class EpicsLatexLinkChains
{

};

#endif