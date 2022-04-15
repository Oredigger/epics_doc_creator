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

// The command argument will look like the following
// ./main --files test_files/sample7.db test_files/sample1.db --subs test_files/x.sub
//

int main(int argc, char *argv[])
{ 
    //if (argc <= 1)
    //    return EXIT_FAILURE;

    EpicsDbFileLexAnalysis x("test_files/example.db");
    x.parse_dft();

    q_token q_state = x.get_q_state();
    x.print_q_state();
    
    EpicsRecordChain y(q_state);
    y.print_adj_mat();
    y.traverse(0, 0);
    
    EpicsLatexGen gen("test_files/example.db", q_state);
    gen.save_as_file("test");
    
    return EXIT_SUCCESS;
}
