#include <iostream>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <tuple>

#include "epics_latex_gen.hpp"
#include "parse_util.hpp"

static std::string check_str_in_latex(std::string r_str, bool in_cmd)
{
    replace_all_substr(r_str,  "\\", "\\textbackslash ");
    replace_all_substr(r_str,   "_", "\\_ ");
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

    /* When in_cmd is set to be TRUE, then that means r_str is bounded by parenthesis.
     In that scenario, we have to make sure that the modification made to r_str is 
     valid. */

    replace_all_substr(r_str,  "\n", in_cmd ? "\\\\" : "\n");

    return r_str;
}

std::string gen_header_str(q_token &q_state)
{
    /* A header string is generated from the token sequences, which will then be converted into a 
    latex conforming string in a later function. */

    bool is_header = false;

    std::string comment;
    lex_states prev_state;

    while (!q_state.empty())
    {
        auto elem = q_state.front();

        lex_states state = std::get<0>(elem);
        std::string name = std::get<1>(elem);

        trim_whitespace(name);

        if (state == COMMENT)
        {
            if (name == "##")
            {
                if (is_header)
                {
                    q_state.pop();
                    break;
                }

                is_header = true;
            }
            else if (is_header)
            {
                comment += name;

                if (name.back() != ' ')
                    comment += ' ';
            }
        }
        else if (state == NEWLINE)
        {
            if (prev_state == POUND)
                comment += '\n';
        }
        else if (state != POUND) 
        {
            break;
        }
        
        prev_state = state;
        q_state.pop();
    }

    return comment;
}

EpicsLatexHeader::EpicsLatexHeader(void)
{}

EpicsLatexHeader::EpicsLatexHeader(std::string h_str)
{
    load(h_str);
}

void EpicsLatexHeader::load(std::string h_str)
{
    std::string results[] = {"", "", "", "", "", "", ""};
    int i = 0;

    for (auto p : PARAM_KEYS)
    {
        std::string temp;
        size_t p_idx = h_str.find("/" + p);

        if (p_idx != std::string::npos)
        {
            size_t start_idx = p_idx + p.length() + 2;
            size_t end_idx = h_str.find("/", start_idx);

            if (end_idx == std::string::npos)
                end_idx = h_str.length() - 1;

            if (end_idx != std::string::npos)
            {
                size_t s_str_len = end_idx - start_idx;
                temp = h_str.substr(start_idx, s_str_len);
            }
        }

        results[i++] = temp;
    }
    
    author = results[0];
    brief  = results[1];
    bug    = results[2];
    desc   = results[3];
    file   = results[4];
    param  = results[5];
    remark = results[6];
}

void EpicsLatexHeader::clear(void)
{
    conv   = "";
    author = "";
    brief  = "";
    bug    = "";
    desc   = "";
    file   = "";
    param  = "";
    remark = "";
}

std::string EpicsLatexFileHeader::get_latex_str(void)
{
    if (!file.empty())
    {
        conv += "\\section{" + check_str_in_latex(file, true) + "}\n";

        if (!brief.empty())
            conv += "\\textit{" +  check_str_in_latex(brief, true) + "}\n";

        if (!desc.empty())
            conv += "\\subsection{Description}\n" + check_str_in_latex(desc, false) + "\n";

        if (!bug.empty())
            conv += "\\subsection{Bug Notes}\n" + check_str_in_latex(bug, false) + "\n";
    }
    else
    {
        conv += "\\section{}\n";
    }

    return conv;
}

std::string EpicsLatexRecordHeader::get_latex_str(void)
{
    if (!brief.empty())
        conv += "\\subsubsection{Brief}\n" + check_str_in_latex(brief, true) + "\n";

    if (!desc.empty())
        conv += "\\subsubsection{Description}\n" + check_str_in_latex(desc, false) + "\n";

    if (!bug.empty())    
        conv += "\\subsubsection{Bug Notes}\n" + check_str_in_latex(bug, false) + "\n";

    return conv;
}

std::string EpicsLatexRecordParam::get_latex_str(void)
{
    if (!param.empty())
        conv += "\\textit{" + check_str_in_latex(param, true) + "}\\\\\\\\ \n";

    return conv;
}

EpicsLatexRecordBody::EpicsLatexRecordBody(void)
{}

EpicsLatexRecordBody::EpicsLatexRecordBody(q_token &q_state)
{
    load(q_state);
}

void EpicsLatexRecordBody::load(q_token &q_state)
{
    bool is_record = false, is_comment = false;
    std::string field_type, comment;

    while (!q_state.empty())
    {
        auto elem  = q_state.front();
        lex_states state = std::get<0>(elem);
        std::string name = std::get<1>(elem);
    
        if (state == HEADER)
        {
            is_record = name == "record";
        }
        else if (state == TYPE)
        {
            if (is_record)
                record_type = name;
            else
                field_type = name;
        }
        else if (state == VALUE)
        {
            if (is_record)
                record_name = name;
            else
                field_value_m[field_type] = name;
        }
        else if (state == RIGHT_PAREN)
        {
            if (!comment.empty())
            {
                field_comment_m[field_type] = comment;
                comment = "";
            }
        }
        else if (state == COMMENT)
        {
            if (name == "##")
            {
                if (is_comment && name.back() != ' ')
                    comment += ' ';
        
                is_comment = !is_comment;
            }
            else if (is_comment)
                comment += name;
        }
        else if (state == RIGHT_CURLY)
        {
            break;
        }

        q_state.pop();
    }
}

void EpicsLatexRecordBody::clear(void)
{
    field_value_m.clear();
    field_comment_m.clear();
    
    conv = "";
    record_name = "";
    record_type = "";
}

std::string EpicsLatexRecordBody::get_latex_str(void)
{
    conv += "\\newpage\n";
    conv += "\\subsection{" + check_str_in_latex(record_name, true) + " - " + "\\textit{" + check_str_in_latex(record_type, true) + "}}\n";
    conv += "\\subsubsection{Fields}\n";

    for (const auto& p : field_value_m)
    {
        conv += check_str_in_latex(p.first, false) + ": " + check_str_in_latex(p.second, false) + "\\\\\\\\ ";

        if (field_comment_m.find(p.first) != field_comment_m.end())
        {
            EpicsLatexRecordParam r_param(field_comment_m[p.first]);
            conv += r_param.get_latex_str();
        }
    }

    return conv;
}

EpicsLatexGen::EpicsLatexGen(std::string tex_fn, std::string db_fn, q_token q_state)
{
    latex += DOC_HEADER + FILE_BEGIN;
    std::string h_str = gen_header_str(q_state);
    
    EpicsLatexFileHeader f_header(h_str); 
    latex += f_header.get_latex_str();
    
    bool is_record = false;
    EpicsLatexRecordHeader r_header;
    EpicsLatexRecordBody r_body;

    while (!q_state.empty())
    {
        auto elem = q_state.front();

        lex_states state = std::get<0>(elem);
        std::string name = std::get<1>(elem);

        trim_whitespace(name);

        if (state == HEADER && name == "record")
        {
            r_body.load(q_state);
            is_record = true;
            
            continue;
        }

        if (state == COMMENT)
        {
            if (name == "##")
            {
                std::string r_str = gen_header_str(q_state);
                r_header.load(r_str);
            }

            if (name == "}")
                r_header.clear();
        }
        
        if (state == RIGHT_CURLY)
        {
            if (is_record)
            {
                std::string latex_r_header = r_header.get_latex_str();
                std::string latex_r_body = r_body.get_latex_str();

                size_t insert_idx = latex_r_body.find("}}") + 3;
                latex_r_body.insert(insert_idx, latex_r_header);
                latex += latex_r_body;
            }

            is_record = false;

            r_header.clear();
            r_body.clear();
        }

        q_state.pop();
    }

    latex += FILE_END;
    struct stat buf;
    
    if (stat("./tex", &buf)) mkdir("./tex", 755);

    std::ofstream fout;
    fout.open("./tex/" + tex_fn + ".tex", std::ofstream::out);

    if (fout.good()) 
        fout << latex;  

    fout.close();
}