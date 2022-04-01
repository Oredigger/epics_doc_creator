#include <iostream>
#include <fstream>
#include <tuple>

#include "epics_latex_gen.hpp"

static const std::string DOC_HEADER = "\\documentclass[12pt]{article}\n";
static const std::string FILE_BEGIN = "\\begin{document}\n";
static const std::string FILE_END   = "\\end{document}\n";

void gen_latex_doc(std::string fn, q_token q_state)
{
    std::ofstream fout;
    fout.open(fn + ".tex", std::ofstream::out);

    if (!fout.good())
        return;

    fout << DOC_HEADER;
    fout << FILE_BEGIN;

    size_t header_state = 0;

    while (!q_state.empty())
    {
        auto elem = q_state.front();
        dpl_states state = std::get<0>(elem);
        std::string name = std::get<1>(elem);
        
        // Make sure to convert _ to /_
    
        switch (state)
        {
            case HEADER:
                if (name == "record")
                {
                    fout << "\\section";
                    header_state = 0;
                }
                if (name == "field")
                    header_state = 1;

                break;
            case TYPE:
                if (header_state)
                    fout << "\\textbf{" << name << ": }";

                break;
            case VALUE:
                if (header_state)
                    fout << name << " \\\\";
                else
                    fout << "{" << name << "}";

                fout << "\n";
                break;
            case RIGHT_CURLY:
                fout << "\\newpage\n";
        }

        q_state.pop();
    }
    
    fout << FILE_END;  
    fout.close();
}