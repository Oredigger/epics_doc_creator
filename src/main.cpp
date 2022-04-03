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

    for (int i = 0; i < argc - 1; i++)
    {
        std::cout << argv[i + 1] << std::endl;
    }

    EpicsLexAnalysis x(argv[1]);
    x.print_q_state();
    gen_latex_doc("hmm", argv[1], x.get_q_state());
    
    return EXIT_SUCCESS;
}