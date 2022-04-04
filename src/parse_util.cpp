#include <algorithm>
#include <cctype>
#include "parse_util.hpp"
#include <iostream>

void remove_all_char(std::string &r_str, char c)
{
    r_str.erase(std::remove(r_str.begin(), r_str.end(), c), r_str.end());
}

void trim_whitespace(std::string &r_str)
{
    int idx_0, idx_1;

    for (idx_0 = 0; idx_0 < r_str.length(); idx_0++)
    {
        if (!isspace(r_str[idx_0]))
            break;
    }

    r_str.erase(0, idx_0);

    for (idx_1 = r_str.length() - 1; idx_1 >= idx_0; idx_1--)
    {
        if (!isspace(r_str[idx_1]))
            break;
    }

    r_str.erase(idx_1 + 1, r_str.length() - idx_1);
}

void replace_all_char(std::string &r_str, char c, char s)
{
    r_str.replace(r_str.begin(), r_str.end(), c, s);
}

void replace_all_substr(std::string &r_str, std::string target, std::string replace)
{
    // I was lazy, so I took the example code from Varun of thispointer.com
    size_t first_occ = r_str.find(target);

    while (first_occ != std::string::npos)
    {
        r_str.replace(first_occ, target.size(), replace);
        first_occ = r_str.find(target, first_occ + replace.length());
    }
}

std::queue<size_t> get_all_char_pos(std::string r_str, char c)
{
    std::queue<size_t> q_idx;

    for (size_t i = 0; i < r_str.length(); i++)
    {
        if (r_str[i] == c)
            q_idx.push(i);
    }

    return q_idx;
}

std::string prep_r_str(std::string r_str)
{
    remove_all_char(r_str, '\t');
    remove_all_char(r_str, '\r');

    // Do not remove spaces in substrings that are surrounded by quotes.
    std::queue<size_t> q_quote_loc = get_all_char_pos(r_str, '"');
    bool is_comment = false;

    std::string f_str;
    size_t q_idx_0 = 0, q_idx_1 = 0;

    // Not the most optimal solution - however this prevents memory leaks and unconditional branching from 
    // occurring though!
    for (size_t i = 0; i < r_str.length(); i++)
    {
        if (r_str[i] == '#')
            is_comment = true;
        else if (r_str[i] == '\n')
            is_comment = false;

        if (i == q_idx_1)
        {
            if (q_quote_loc.empty())
            {
                q_idx_1 = q_idx_0;
            }
            else
            {
                q_idx_0 = q_quote_loc.front();
                q_quote_loc.pop();

                q_idx_1 = q_quote_loc.front();
                q_quote_loc.pop();
            }
        }

        if (!is_comment)
        {
            if ((i <= q_idx_0 || i >= q_idx_1) && r_str[i] != ' ')
                f_str += r_str[i];
            else if (i > q_idx_0 && i < q_idx_1)
                f_str += r_str[i];
        }
        else
        {
            f_str += r_str[i];
        }
    }

    return f_str;
}