#ifndef EPICS_LEX_ANALYSIS_H
#define EPICS_LEX_ANALYSIS_H

#include <map>
#include <string>
#include <queue>

enum lex_states
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
    AT,
    POUND,
    COMMENT,
    INVALID,
};

typedef std::queue<std::tuple<lex_states, std::string, size_t>> q_token;

class EpicsLexAnalysis
{
    public:
        EpicsLexAnalysis(void);
        EpicsLexAnalysis(std::string fn);

        q_token get_q_state(void);
        void print_q_state(void);
        void load_file(std::string fn);
        
    private:
        q_token q_state;
};

#endif