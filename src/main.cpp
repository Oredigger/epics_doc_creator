// STL libraries
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

// Project libraries
#include "epics_db_parse.hpp"
#include "epics_latex_gen.hpp"

int main(int argc, char *argv[])
{
    EPICS_DB_parse x("./test_files/sample3.db");
    x.print_q_state();
    gen_latex_doc("hmm", x.get_q_state());

    return 0;
}