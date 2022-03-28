// STL libraries
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

// Project libraries
#include "epics_dbd_parse.h"
#include "latex_generator.h"

int main(int argc, char *argv[])
{
    EPICS_DBD_parse x("sample.dbd");

    std::ofstream fout;
    fout.open("doc.tex", std::ofstream::out);

    if (fout.good())
        fout << FILE_HEADER;
    
    fout.close();
    return 0;
}