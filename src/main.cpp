// STL libraries
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

// Project libraries
#include "epics_lex_analysis.hpp"
#include "epics_latex_gen.hpp"
#include "epics_record_chain.hpp"

int main(int argc, char *argv[])
{
    //if (argc <= 1)
    //    return EXIT_FAILURE;

    EpicsLexAnalysis x("test_files/sample2.db");
    q_token q_state = x.get_q_state();
    
    EpicsRecordChain y(q_state);
    y.print_adj_mat();

    //EpicsLatexGen gen(argv[2], argv[1], q_state);
    return EXIT_SUCCESS;
}
