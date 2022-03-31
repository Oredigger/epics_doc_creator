// STL libraries
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

// Project libraries
#include "epics_db_parse.hpp"
#include "latex_gen.hpp"

int main(int argc, char *argv[])
{
    //EPICS_DBD_parse x("sample.dbd");
    EPICS_DB_parse x("./test_files/sample.db");

    std::ofstream fout;
    fout.open("doc.tex", std::ofstream::out);

    if (fout.good())
        fout << FILE_HEADER;
    
    fout.close();
    return 0;
}