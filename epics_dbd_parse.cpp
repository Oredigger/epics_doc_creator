#include "epics_dbd_parse.h"
#include "parse_util.h"

#include <fstream>
#include <iostream>
#include <sstream>

enum dbd_parse_lexer_states
{
    menutype,
    include,
    path,
    addpath,
    recordtype,
    choice,
    choice_name,
    choice_value,
    field,
    field_name,
    field_value,
    left_paren,
    right_paren,
    left_curly,
    right_curly,
    comma,
    double_quote
};

EPICS_DBD_parse::EPICS_DBD_parse(void)
{
    std::cout << "Hello world!" << std::endl;
}

EPICS_DBD_parse::EPICS_DBD_parse(std::string fn)
{
    std::string r_str;
    std::ifstream fin;

    fin.open(fn, std::ifstream::in);

    if (fin.good())
    {
        std::ostringstream os;

        if (os.good())
            os << fin.rdbuf() << std::endl;
        
        r_str = os.str();
        remove_whitespace(r_str);
    }

    std::cout << r_str << std::endl;
    fin.close();
}