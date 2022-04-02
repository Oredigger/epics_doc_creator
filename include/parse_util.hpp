#ifndef PARSE_UTIL_H
#define PARSE_UTIL_H

#include <string>
#include <queue>

void remove_all_char(std::string &r_str, char c);
void replace_all_char(std::string &r_str, char c, char s);

std::queue<size_t> get_all_char_pos(std::string r_str, char c);
std::string prep_r_str(std::string r_str);

#endif