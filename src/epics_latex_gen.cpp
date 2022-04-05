#include <iostream>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <tuple>

#include "epics_latex_gen.hpp"
#include "parse_util.hpp"

static const std::string DOC_HEADER = "\\documentclass[12pt]{article}\n";
static const std::string FILE_BEGIN = "\\begin{document}\n";
static const std::string FILE_END   = "\\end{document}\n";

const std::string PARAM_KEYS[] = 
{
    "author",
    "brief", 
    "bug", 
    "desc",
    "file",
    "endlink",
    "link",
    "param",
    "remark",
    "short",
}; 

const int AUTHOR_KEY  = 0;
const int BRIEF_KEY   = 1;
const int BUG_KEY     = 2;
const int DESC_KEY    = 3;
const int FILE_KEY    = 4;
const int ENDLINK_KEY = 5;
const int LINK_KEY    = 6;
const int PARAM_KEY   = 7;
const int REMARK_KEY  = 8;
const int SHORT_KEY   = 9;

static void check_str_in_latex(std::string &r_str, bool in_cmd)
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
    replace_all_substr(r_str,  "\n", in_cmd ? "\\" : "\n");
}

EpicsLatexHeader::EpicsLatexHeader(std::string dbh_str)
{
    const size_t H_KEYS_LEN = 4;
    const int HEADER_KEYS[H_KEYS_LEN] = 
    {
        FILE_KEY, 
        BRIEF_KEY, 
        BUG_KEY,
        DESC_KEY
    };

    std::string results[] = {"", "", "", ""};

    for (int i = 0; i < H_KEYS_LEN; i++)
    {
        std::string temp;
        std::string param = PARAM_KEYS[HEADER_KEYS[i]];
        
        size_t p_idx = dbh_str.find("/" + param);

        if (p_idx != std::string::npos)
        {
            size_t start_idx = p_idx + param.length() + 1;
            size_t end_idx = dbh_str.find((i == H_KEYS_LEN - 1) ? "\n##" : "/", start_idx);

            if (end_idx != std::string::npos)
            {
                size_t s_str_len = end_idx - start_idx;
                temp = dbh_str.substr(start_idx, s_str_len);
            }
        }

        results[i] = temp;
    }
    
    file  = results[0];
    brief = results[1];
    bug   = results[2];
    desc  = results[3];
}

std::string EpicsLatexHeader::get_conv_str(void)
{
    check_str_in_latex(file, true);
    conv += "\\section{" + file + "}\n";

    check_str_in_latex(brief, true);
    conv += "\\textit{" + brief + "}\n";

    check_str_in_latex(desc, false);
    conv += "\\subsection{Description}\n" + desc + "\n";

    check_str_in_latex(bug, false);
    conv += "\\subsection{Bug Notes}\n" + bug + "\n";

    return conv;
}

std::string gen_header_str(std::string db_fn, q_token &q_state)
{
    std::string comment, name;
    lex_states state;
    bool is_header = false;

    while (!q_state.empty())
    {
        auto elem = q_state.front();
        state = std::get<0>(elem);
        name  = std::get<1>(elem);
        
        if (state == COMMENT)
        {
            if (is_header)
                comment += name;

            if (name == "##")
            {
                if (is_header)
                {
                    q_state.pop();
                    break;
                }
                is_header = true;
            }
        }
        else if (state == NEWLINE)
            comment += '\n';
        else if (state != POUND)
            break;
        
        q_state.pop();
    }

    return comment;
}

void gen_latex_doc(std::string tex_fn, std::string db_fn, q_token q_state)
{
    struct stat buf;
    
    if (stat("./tex", &buf))
        mkdir("./tex", 777);

    std::ofstream fout;
    fout.open("./tex/" + tex_fn + ".tex", std::ofstream::out);

    if (!fout.good())
        return;

    std::string base_fn = db_fn.substr(db_fn.find_last_of("/") + 1);
    fout << DOC_HEADER << FILE_BEGIN;
   
    std::string dbh_str = gen_header_str(db_fn, q_state);
    EpicsLatexHeader db_file_head(dbh_str); 
    fout << db_file_head.get_conv_str();

    bool header_state = 0;
    std::string name;
    lex_states state;

    if (!q_state.empty())
        fout << "\\subsection{Records}\n";

    print_q_state(q_state);
    
    /*while (!q_state.empty())
    {
        auto elem  = q_state.front();
        state = std::get<0>(elem);
        name  = std::get<1>(elem);
    
        if (state == HEADER)
        {
            if (name == "record")
            {
                fout << "\\subsubsection";
                header_state = 0;
            }
            if (name == "field")
                header_state = 1;
        }
        else if (state == TYPE)
        {
            if (header_state)
            {
                check_str_in_latex(name, true);
                fout << "\\textbf{" << name << ": }";
            }
        }
        else if (state == VALUE)
        {
            check_str_in_latex(name, false);

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
    }*/
    
    fout << FILE_END;  
    fout.close();
}