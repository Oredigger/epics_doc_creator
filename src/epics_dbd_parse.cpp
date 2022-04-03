#include "epics_dbd_parse.hpp"
#include "parse_util.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

EpicsDbdParse::EpicsDbdParse(void)
{
    std::cout << "Hello world!" << std::endl;
}

EpicsDbdParse::EpicsDbdParse(std::string fn)
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
        
        remove_all_char(r_str, '\t');
        remove_all_char(r_str, ' ');
        remove_all_char(r_str, '\r');
    }

    std::cout << r_str << std::endl;
    fin.close();
}