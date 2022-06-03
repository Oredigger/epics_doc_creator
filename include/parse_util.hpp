#ifndef PARSE_UTIL_H
#define PARSE_UTIL_H

#include <string>
#include <queue>

void remove_all_char(std::string &r_str, char c);
void trim_whitespace(std::string &r_str);
void replace_all_char(std::string &r_str, char c, char s);
void replace_all_substr(std::string &r_str, std::string target, std::string replace);

std::queue<size_t> get_all_char_pos(std::string r_str, char c);

#endif