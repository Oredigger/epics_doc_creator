// STL libraries
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

// Project libraries
#include "epics_lex_analysis.hpp"
#include "epics_latex_gen.hpp"
#include "epics_record_chain.hpp"

// The command argument will look like the following
// ./main --input test_files/sample7.db test_files/sample1.db --output test_files/x.sub

int main(int argc, char *argv[])
{ 
    if (argc <= 1)
        return EXIT_FAILURE;

    // If read_files is set to be a 1, then read in the input files.
    // If read_files is set to be a 2, then read in the output file.
    // Otherwise, do nothing.

    char read_files = 0;
    std::queue<std::string> input_names;
    std::string output_name;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--input") == 0)
            read_files = 1;
        else if (strcmp(argv[i], "--output") == 0)
            read_files = 2;
        else
        {
            if (read_files == 1)
                input_names.push(argv[i]);
            else if (read_files == 2)
                output_name = argv[i];
        }
    }

    std::string r_str;

    while (!input_names.empty())
    {
        std::ifstream fin(input_names.front());

        if (fin.good())
        {
            std::ostringstream os;

            if (os.good())
            {
                os << fin.rdbuf() << std::endl;
                r_str += os.str();
            }
        }

        input_names.pop();
        fin.close();
    }

    EpicsDbFileLexAnalysis x;
    x.set_r_str(r_str);
    x.parse_dft();

    q_token q_state = x.get_q_state();
    x.print_q_state();
    
    EpicsRecordChain y(q_state);
    y.print_adj_mat();
    y.create_visual_graph(output_name);
    
    //EpicsLatexGen gen("test_files/example.db", q_state);
    //gen.save_as_file("test");

    return EXIT_SUCCESS;
}
