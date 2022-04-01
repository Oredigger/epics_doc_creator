#ifndef EPICS_DB_PARSE_H
#define EPICS_DB_PARSE_H

#include <map>
#include <string>
#include <queue>

enum dpl_states
{
    HEADER,
    TYPE,
    VALUE,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_CURLY,
    RIGHT_CURLY,
    COMMA,
    DOUBLE_QUOTE,
    NEWLINE,
    POUND,
    COMMENT,
    INVALID,
    START
};

typedef std::queue<std::tuple<dpl_states, std::string, size_t>> q_token;

class EPICS_DB_parse
{
    private:
        q_token q_state;
    public:
        EPICS_DB_parse(void);
        EPICS_DB_parse(std::string fn);

        q_token get_q_state(void);
        void print_q_state(void);
};

short check_calc_eq(std::string pinfix);

#endif