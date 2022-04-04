#ifndef EPICS_LATEX_GEN_H
#define EPICS_LATEX_GEN_H

#include <string>
#include "epics_lex_analysis.hpp"

class EpicsLatexDbHeader
{
    public:
        EpicsLatexDbHeader(void);
        EpicsLatexDbHeader(std::string dbh_str);
        void load_db_header_str(std::string dbh_str);
        std::string get_conv_str(void);

    private:
        std::string conv;
        std::string brief;
        std::string bug;
        std::string desc;
        std::string file;
};

class EpicsLatexLinkChains
{

};

std::string gen_dbh_str(std::string db_fn, q_token q_state);
void gen_latex_doc(std::string tex_fn, std::string db_fn, q_token q_state);

#endif