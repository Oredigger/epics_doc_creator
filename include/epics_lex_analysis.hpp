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
    PERCENT,
    C_CODE,
    INVALID,
};

typedef std::queue<std::tuple<lex_states, std::string, size_t>> q_token;

class EpicsLexAnalysis
{
    public:
        EpicsLexAnalysis(std::string fn);
        EpicsLexAnalysis(void);

        q_token get_q_state(void);
        void print_q_state(void);
        virtual void parse_dft(void) = 0;
        
    protected:
        q_token q_state;
        std::string f_str;
        std::string r_str;
};

class EpicsDbFileLexAnalysis : public EpicsLexAnalysis
{
    public:
        EpicsDbFileLexAnalysis(std::string fn) : EpicsLexAnalysis(fn){};
        EpicsDbFileLexAnalysis(void) : EpicsLexAnalysis(){};
        void parse_dft(void);
    private:
        void prep_r_str(void);
};

class EpicsTempFileLexAnalysis : public EpicsLexAnalysis
{
    public:
        EpicsTempFileLexAnalysis(std::string fn) : EpicsLexAnalysis(fn){};
        EpicsTempFileLexAnalysis(void) : EpicsLexAnalysis(){};
        void parse_dft(void);
    private:
        void prep_r_str(void);
};


#endif