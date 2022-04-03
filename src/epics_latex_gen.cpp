#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <tuple>

#include "epics_latex_gen.hpp"
#include "parse_util.hpp"

static const std::string DOC_HEADER = "\\documentclass[12pt]{article}\n";
static const std::string FILE_BEGIN = "\\begin{document}\n";
static const std::string FILE_END   = "\\end{document}\n";

static void check_str_in_latex(std::string &r_str)
{
    replace_all_substr(r_str,  "\\", "\\textbackslash ");
    replace_all_substr(r_str,   "_", "\\textunderscore ");
    replace_all_substr(r_str,   "¶", "\\P ");
    replace_all_substr(r_str,   "^", "\\textasciicircum ");
    replace_all_substr(r_str,   "$", "\\$ ");
    replace_all_substr(r_str,   "§", "\\S ");
    replace_all_substr(r_str,   "~", "\\textasciitilde ");
    replace_all_substr(r_str,   "{", "\\{ "); 
    replace_all_substr(r_str, "...", "\\dots ");
    replace_all_substr(r_str,   "<", "\\textless ");
    replace_all_substr(r_str,   ">", "\\greater ");
    replace_all_substr(r_str,   "/", "\\slash ");
}

void gen_latex_doc(std::string tex_fn, std::string db_fn, q_token q_state)
{
    std::ofstream fout;
    struct stat buf;
    
    if (stat("./tex", &buf))
    {
        std::cout << "Creating tex directory" << std::endl;
        mkdir("./tex", 777);
    }

    fout.open("./tex/" + tex_fn + ".tex", std::ofstream::out);

    if (!fout.good())
        return;

    fout << DOC_HEADER;
    fout << FILE_BEGIN;

    size_t header_state = 0;
    std::string base_fn = db_fn.substr(db_fn.find_last_of("/") + 1);
    std::cout << base_fn << std::endl;

    while (!q_state.empty())
    {
        auto elem = q_state.front();
        lex_states state = std::get<0>(elem);
        std::string name = std::get<1>(elem);
        
        // Make sure to convert _ to /_
    
        if (state == HEADER)
        {
            if (name == "record")
            {
                fout << "\\section";
                header_state = 0;
            }
            if (name == "field")
                header_state = 1;
        }
        else if (state == TYPE)
        {
            if (header_state)
            {
                check_str_in_latex(name);
                fout << "\\textbf{" << name << ": }";
            }
        }
        else if (state == VALUE)
        {
            check_str_in_latex(name);

            if (header_state)
                fout << name << " \\\\";
            else
                fout << "{" << base_fn << "/" << name << "}";

            fout << "\n";
        }
        else if (state == RIGHT_CURLY)
        {
            fout << "\\newpage\n";
        }

        q_state.pop();
    }
    
    fout << FILE_END;  
    fout.close();
}