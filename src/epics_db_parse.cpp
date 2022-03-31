#include "epics_db_parse.hpp"
#include "parse_util.hpp"

#include <algorithm>
#include <cstring>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

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
    INVALID
};

static std::string state_2_str(dpl_states state)
{
    switch (state)
    {
        case HEADER:
            return "HEADER";
        case TYPE:
            return "TYPE";
        case VALUE:
            return "VALUE";
        case LEFT_PAREN:
            return "LEFT_PAREN";
        case RIGHT_PAREN:
            return "RIGHT_PAREN";
        case LEFT_CURLY:
            return "LEFT_CURLY";
        case RIGHT_CURLY:
            return "RIGHT_CURLY";
        case COMMA:
            return "COMMA";
        case DOUBLE_QUOTE:
            return "DOUBLE_QUOTE";
        case NEWLINE:
            return "NEWLINE";
    }

    return "INVALID";
}

void next_state(dpl_states &state, char next)
{
    switch (next)
    {
        case '(':
            state = LEFT_PAREN;
            break;
        case ')':
            state = RIGHT_PAREN;
            break;
        case '{':
            state = LEFT_CURLY;
            break;
        case '}':
            state = RIGHT_CURLY;
            break;
        case ',':
            state = COMMA;
            break;
        case '"':
            state = DOUBLE_QUOTE;
            break;
        case '\n':
            state = NEWLINE;
            break;
        default:
            if (state != VALUE)
                state = INVALID;
            break;
    }
}

typedef std::queue<std::tuple<dpl_states, std::string, size_t>> q_token;

static void str_state(q_token &q_state, dpl_states &state, std::string &token, char next, char curr, size_t line_num)
{
    token += curr;

    if (!isalpha(next) && !isdigit(next) && next != '_')
    {
        q_state.push(std::make_tuple(state, token, line_num));
        next_state(state, next);
        token.clear();
    }
}

static void ch_state(q_token &q_state, dpl_states &state, dpl_states str_state, char next, char curr, size_t line_num)
{
    q_state.push(std::make_tuple(state, std::string(1, curr), line_num));
                
    if (isalpha(next) || isdigit(next) || next == '_')
        state = str_state;
    else
        next_state(state, next);
}

static std::queue<DB_record_inst> parse_dft(std::string r_str)
{
    std::queue<DB_record_inst> q_inst;
    q_token q_state;

    remove_all_char(r_str, '\t');
    remove_all_char(r_str, ' ');
    remove_all_char(r_str, '\r');

    std::string token;
    dpl_states curr_state = HEADER;
    
    size_t line_num = 1;
    r_str += ' ';

    for (size_t i = 0; i < r_str.length(); i++)
    {
        char curr = r_str[i];
        char next = r_str[i + 1];

        switch (curr_state)
        {
            case HEADER:
                str_state(q_state, curr_state, token, next, curr, line_num);
                break;
            
            case TYPE:
                str_state(q_state, curr_state, token, next, curr, line_num);
                break;

            case VALUE:
                str_state(q_state, curr_state, token, next, curr, line_num);
                break;
            
            case LEFT_PAREN:
                ch_state(q_state, curr_state, TYPE, next, curr, line_num);
                break;
            
            case RIGHT_PAREN:
                ch_state(q_state, curr_state, HEADER, next, curr, line_num);
                break;
            
            case LEFT_CURLY:
                ch_state(q_state, curr_state, HEADER, next, curr, line_num);
                break;
            
            case RIGHT_CURLY:
                ch_state(q_state, curr_state, HEADER, next, curr, line_num);
                break;
            
            case COMMA:
                ch_state(q_state, curr_state, VALUE, next, curr, line_num);
                break;

            case DOUBLE_QUOTE:
                ch_state(q_state, curr_state, VALUE, next, curr, line_num);
                break;

            case NEWLINE:
                ch_state(q_state, curr_state, HEADER, next, curr, line_num);
                line_num++;

                break;

            default:
                // Account for stray characters here.
                token += curr;
                
                if (isalpha(next) || isdigit(next) || next == '_')
                    curr_state = HEADER;

                break;
        }
    }
         
    while (!q_state.empty())
    {
        auto elem = q_state.front();
        std::string token = (std::get<0>(elem) == NEWLINE) ? 
                            "\\n" : std::get<1>(elem);

        std::cout << state_2_str(std::get<0>(elem)) << "  " << token << "  " << std::get<2>(elem) << std::endl;
        q_state.pop();
    }

    return q_inst;
}

EPICS_DB_parse::EPICS_DB_parse(void)
{
    q_inst = std::queue<DB_record_inst>();
}

EPICS_DB_parse::EPICS_DB_parse(std::string fn)
{
    std::string r_str;
    std::ifstream fin;

    fin.open(fn, std::ifstream::in);

    if (fin.good())
    {
        std::ostringstream os;

        if (os.good())
            os << fin.rdbuf() << std::endl;
        
        r_str = os.str();
    }

    fin.close();
    q_inst = parse_dft(r_str);
}

/*short check_calc_eq(std::string pinfix)
{
    short error = CALC_ERR_NONE;
    char *p = (char *) calloc(INFIX_TO_POSTFIX_SIZE(pinfix.length()) + 1, sizeof(char));

    if (!p)
        return 0;

    postfix(pinfix.c_str(), p, &error);
    free(p);

    return error;
}*/