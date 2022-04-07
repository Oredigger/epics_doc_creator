// STL libraries
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

// Project libraries
#include "epics_lex_analysis.hpp"
#include "epics_latex_gen.hpp"

int main(int argc, char *argv[])
{
    if (argc <= 1)
        return EXIT_FAILURE;

    EpicsLexAnalysis x(argv[1]);
    q_token q_state = x.get_q_state();
    print_q_state(q_state);
    EpicsLatexGen gen(argv[2], argv[1], q_state);
    
    return EXIT_SUCCESS;
}