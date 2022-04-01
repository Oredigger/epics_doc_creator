#include <algorithm>
#include "parse_util.hpp"

void remove_all_char(std::string &r_str, char c)
{
    r_str.erase(std::remove(r_str.begin(), r_str.end(), c), r_str.end());
}

void replace_all_char(std::string &r_str, char c, char s)
{
    r_str.replace(r_str.begin(), r_str.end(), c, s);
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