#ifndef EPICS_LATEX_GEN_H
#define EPICS_LATEX_GEN_H

#include <string>
#include "epics_lex_analysis.hpp"

const std::string param_keywords[] = 
{
    "brief", 
    "bug", 
    "file",
    "endlink",
    "link",
    "page"
    "remark",
    "short",
}; 

class EpicsLatexRecordSection
{
    public:
    private:
};

class EpicsLatexLinkChains
{

};

void gen_latex_doc(std::string tex_fn, std::string db_fn, q_token q_state);

#endif