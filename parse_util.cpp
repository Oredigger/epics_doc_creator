#include <algorithm>
#include "parse_util.h"
#include <cstring>

void remove_whitespace(std::string &r_str)
{
    char ch_unneeded[] = "   ";
    size_t num_items = strlen(ch_unneeded);

    for (size_t i = 0; i < num_items; i++)
    {
        r_str.erase(std::remove(r_str.begin(), r_str.end(), ch_unneeded[i]), r_str.end());
    }
}