#ifndef EPICS_LATEX_GEN_H
#define EPICS_LATEX_GEN_H

#include <string>
#include "epics_lex_analysis.hpp"

const std::string param_keywords[] = 
{
    "author",
    "brief", 
    "bug", 
    "file",
    "endlink",
    "link",
    "param",
    "remark",
    "short",
}; 

class EpicsLatexDbHeader
{
    public:
        EpicsLatexDbSection(void);
        EpicsLatexDbSection(std::string r_str);

    private:
        std::string author;
        std::string brief;
        std::string bug;
        std::string file;
};

class EpicsLatexLinkChains
{

};

void gen_latex_doc(std::string tex_fn, std::string db_fn, q_token q_state);

#endif