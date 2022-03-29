#include "epics_db_parse.h"
#include "parse_util.h"

#include <algorithm>
#include <cstring>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

enum db_parse_lexer_states
{
    RECORD_HEADER,
    RECORD_TYPE,
    RECORD_NAME,
    FIELD_HEADER,
    FIELD_NAME,
    FIELD_VALUE,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_CURLY,
    RIGHT_CURLY,
    COMMA,
    DOUBLE_QUOTE,
    INVALID
};

static std::string state_2_str(db_parse_lexer_states state)
{
    switch (state)
    {
        case RECORD_HEADER:
            return "RECORD_HEADER";
        case RECORD_TYPE:
            return "RECORD_TYPE";
        case RECORD_NAME:
            return "RECORD_NAME";
        case FIELD_HEADER:
            return "FIELD_HEADER";
        case FIELD_NAME:
            return "FIELD_NAME";
        case FIELD_VALUE:
            return "FIELD_VALUE";
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
    }

    return "INVALID";
}

void next_state(db_parse_lexer_states &state, char next)
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
        default:
            state = INVALID;
            break;
    }
}

static std::queue<DB_record_inst> parse_dft(std::string r_str)
{
    std::queue<DB_record_inst> q_inst;
    std::queue<std::tuple<db_parse_lexer_states, std::string, size_t>> q_state;

    std::string token;
    db_parse_lexer_states curr_state = RECORD_HEADER;

    bool in_scope = false, in_quote = false, f = false;
    char curr, next;
    size_t line_num = 1;

    remove_all_char(r_str, '\t');
    remove_all_char(r_str, ' ');
    remove_all_char(r_str, '\r');

    std::queue<size_t> q_idx = get_all_char_pos(r_str, '\n');
    remove_all_char(r_str, '\n');

    size_t r_str_len = r_str.length();
    r_str += ' ';

    for (size_t i = 0; i < r_str_len; i++)
    {
        curr = r_str[i];
        next = r_str[i + 1];

        if (i + line_num > q_idx.front())
        {
            q_idx.pop();
            line_num++;
        }

        switch (curr_state)
        {
            case RECORD_HEADER:
                token += curr;

                if (!isalpha(next) && !isdigit(next) && next != '_' && next != '.')
                {
                    q_state.push(std::make_tuple(curr_state, token, line_num));
                    next_state(curr_state, next);
                    token.clear();
                }
                
                break;
            
            case RECORD_TYPE:
                token += curr;

                if (next == ',')
                {
                    q_state.push(std::make_tuple(curr_state, token, line_num));
                    next_state(curr_state, next);
                    token.clear();
                }
                
                break;

            case RECORD_NAME:
                token += curr;
                f = (in_quote) ? next == '"' : next == ')';

                if (f)
                {
                    q_state.push(std::make_tuple(curr_state, token, line_num));
                    next_state(curr_state, next);
                    token.clear();
                }
                
                break;

            case FIELD_HEADER:
                token += curr;

                if (!isalpha(next) && !isdigit(next) && next != '_' && next != '.')
                {
                    q_state.push(std::make_tuple(curr_state, token, line_num));
                    next_state(curr_state, next);
                    token.clear();
                }
                
                break;
            
            case FIELD_NAME:
                token += curr;

                if (next == ',')
                {
                    q_state.push(std::make_tuple(curr_state, token, line_num));
                    next_state(curr_state, next);
                    token.clear();
                }
                
                break;

            case FIELD_VALUE:
                token += curr;
                f = (in_quote) ? next == '"' : !isalpha(next) && !isdigit(next) && next != '_' && next != '.';

                if (f)
                {
                    q_state.push(std::make_tuple(curr_state, token, line_num));
                    next_state(curr_state, next);
                    token.clear();
                }
                
                break;
            
            case LEFT_PAREN:
                q_state.push(std::make_tuple(curr_state, std::string(1, curr), line_num));
                
                if (isalpha(next) || isdigit(next) || next == '_' || next == '.')
                    curr_state = in_scope ? FIELD_NAME : RECORD_TYPE;
                else
                    next_state(curr_state, next);

                break;
            
            case RIGHT_PAREN:
                q_state.push(std::make_tuple(curr_state, std::string(1, curr), line_num));
                
                if (isalpha(next) || isdigit(next) || next == '_' || next == '.')
                    curr_state = in_scope ? FIELD_HEADER : INVALID;
                else
                    next_state(curr_state, next);

                break;
            
            case LEFT_CURLY:
                q_state.push(std::make_tuple(curr_state, std::string(1, curr), line_num));
                in_scope = true;

                if (isalpha(next) || isdigit(next) || next == '_' || next == '.')
                    curr_state = FIELD_HEADER;
                else
                    next_state(curr_state, next);

                break;
            
            case RIGHT_CURLY:
                q_state.push(std::make_tuple(curr_state, std::string(1, curr), line_num));
                in_scope = false;

                if (isalpha(next) || isdigit(next) || next == '_' || next == '.')
                    curr_state = RECORD_HEADER;
                else
                    next_state(curr_state, next);

                break;
            
            case COMMA:
                q_state.push(std::make_tuple(curr_state, std::string(1, curr), line_num));

                if (!isalpha(next) && !isdigit(next) && next != '_' && next != '.')
                    next_state(curr_state, next);
                else
                    curr_state = (in_scope) ? FIELD_VALUE : RECORD_NAME;

                break;

            case DOUBLE_QUOTE:
                q_state.push(std::make_tuple(curr_state, std::string(1, curr), line_num));
                in_quote = !in_quote;

                if (in_quote)
                    curr_state = in_scope ? FIELD_VALUE : RECORD_NAME;
                else
                    next_state(curr_state, next);

                break;

            default:
                // Account for stray characters here.
                if (in_scope)
                {
                    
                }
                else if (in_quote)
                {
                }
                else
                {
                    token += curr;
                    
                    if (isalpha(next) || isdigit(next) || next == '_' || next == '.')
                        curr_state = RECORD_HEADER;
                }

                break;
        }
    }
         
    while (!q_state.empty())
    {
        auto elem = q_state.front();
        std::cout << state_2_str(std::get<0>(elem)) << "  " << std::get<1>(elem) << "  " << std::get<2>(elem) << std::endl;
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