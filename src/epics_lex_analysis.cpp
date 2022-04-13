#include <algorithm>
#include <cstring>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

#include "epics_lex_analysis.hpp"
#include "parse_util.hpp"

static bool is_math_op(char next)
{
    const std::string math_op = "()e-+*/%^><=&|!~?:., ";

    for (auto c : math_op)
    {
        if (next == c)
            return true;
    }

    return false;
}

static void next_state(lex_states &state, char next)
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
        case '@':
            state = AT;
            break;
        case '#':
            state = POUND;
            break;
        case '%':
            state = PERCENT;
            break;
        default:
            if (state != VALUE && state != COMMENT && state != C_CODE) state = INVALID;
            break;
    }
}

static void ch_state(q_token &q_state, lex_states &state, char next, char curr, size_t line_num)
{
    q_state.push(std::make_tuple(state, std::string(1, curr), line_num));

    if (state == POUND || state == PERCENT)
    {
        if (next == '\n')
            state = NEWLINE;
        else
            state = (state == POUND) ? COMMENT : C_CODE;
    }
    else if (state == AT)
    {
        if (isalpha(next) || isdigit(next) || next == '_')
            state = VALUE;
        else
            next_state(state, next);
    }
    else
    {
        if (isalpha(next) || next == '_')
        {
            if (curr == '(')
                state = TYPE;
            else
                state = (curr == ',' || curr == '"') ? VALUE : HEADER;
        }
        else if (isdigit(next) || next == '.' || next == '-')
        {
            state = VALUE;
        }
        else
        {
            next_state(state, next);
        }
    }
}

static void push_state_clear_token(q_token &q_state, lex_states &state, std::string &token, char next, size_t line_num)
{
    auto prev_state = state;
    next_state(state, next);

    if (prev_state != state)
    {
        q_state.push(std::make_tuple(prev_state, token, line_num));
        token.clear();
    }
}

static std::string state_2_str(lex_states state)
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
        case AT:
            return "AT";
        case POUND:
            return "POUND";
        case COMMENT:
            return "COMMENT";
        case PERCENT:
            return "PERCENT";
        case C_CODE:
            return "C_CODE";
        default:
            return "INVALID";
    }
}

EpicsLexAnalysis::EpicsLexAnalysis(void)
{}

EpicsLexAnalysis::EpicsLexAnalysis(std::string fn)
{
    if (!q_state.empty())
    {
        q_token empty;
        q_state.swap(empty);
    }

    std::ifstream fin;
    fin.open(fn, std::ifstream::in);

    if (fin.good())
    {
        std::ostringstream os;

        if (os.good())
        {
            os << fin.rdbuf() << std::endl;
            r_str = os.str();
        }
    }

    fin.close();
}

q_token EpicsLexAnalysis::get_q_state(void)
{
    return q_state;
}

void EpicsLexAnalysis::print_q_state(void)
{
    while (!q_state.empty())
    {
        auto elem = q_state.front();
        std::string token = (std::get<0>(elem) == NEWLINE) ? 
                            "\\n" : std::get<1>(elem);

        std::cout << state_2_str(std::get<0>(elem)) << "  " << token << "  " << std::get<2>(elem) << std::endl;
        
        if (std::get<0>(elem) == NEWLINE)
            std::cout << "\n";

        q_state.pop();
    }
}

std::string EpicsDbFileLexAnalysis::prep_r_str(std::string r_str)
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
        if (r_str[i] == '#' || r_str[i] == '\n')
            is_comment = !is_comment;
    
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
            bool bounds_flag = (i <= q_idx_0 || i >= q_idx_1);

            if ((bounds_flag && r_str[i] != ' ') || !bounds_flag)
                f_str += r_str[i];
        }
        else
        {
            f_str += r_str[i];
        }
    }

    return f_str;
}

void EpicsDbFileLexAnalysis::parse_dft(void)
{
    f_str = prep_r_str(r_str);

    if (f_str.length() < 2) 
        return;

    lex_states curr_state = HEADER;

    if (!isalpha(f_str[0]) && !isdigit(f_str[0]) && f_str[0] != '_')   
        next_state(curr_state, f_str[0]);

    std::string token;
    f_str += ' ';

    bool is_equation = false, is_raw = false;
    size_t line_num = 1;

    for (size_t i = 0; i < f_str.length() - 1; i++)
    {
        char curr = f_str[i], next = f_str[i + 1];

        switch (curr_state)
        {
            case HEADER:
                token += curr;

                if (!isalpha(next) && !isdigit(next) 
                    && next != '_' && next != ':' && !is_raw)
                    push_state_clear_token(q_state, curr_state, token, next, line_num);
                
                break;
            
            case TYPE:
                token += curr;
                
                if (!isalpha(next) && !isdigit(next) && next != '_' && !is_raw)
                {
                    is_equation = (token == "CALC");
                    push_state_clear_token(q_state, curr_state, token, next, line_num);
                }

                break;

            case VALUE:
                token += curr;

                if (is_equation)
                {
                    if (next == '\n' || (!isalpha(next) && !isdigit(next) 
                        && next != ':' && !is_math_op(next)))
                    {
                        push_state_clear_token(q_state, curr_state, token, next, line_num);
                        is_equation = false;
                    }
                }
                else
                {
                    if (!isalpha(next) && !isdigit(next) 
                        && next != '-' && next != '_' && next != '.' 
                        && next != 'e' && next != 'E' && !is_raw)
                        push_state_clear_token(q_state, curr_state, token, next, line_num);
                }

                break;
            
            case LEFT_PAREN:
                if (!is_equation)
                    ch_state(q_state, curr_state, next, curr, line_num);
                
                if (is_equation && curr_state != VALUE)
                    curr_state = VALUE;

                break;
            
            case RIGHT_PAREN:
                if (!is_equation)
                    ch_state(q_state, curr_state, next, curr, line_num);

                if (is_equation && curr_state != VALUE)
                    curr_state = VALUE;
                
                break;
            
            case LEFT_CURLY:
                if (!is_equation)
                    ch_state(q_state, curr_state, next, curr, line_num);

                if (is_equation && curr_state != VALUE)
                    curr_state = VALUE;

                break;
            
            case RIGHT_CURLY:
                if (!is_equation)
                    ch_state(q_state, curr_state, next, curr, line_num);

                if (is_equation && curr_state != VALUE)
                    curr_state = VALUE;

                break;
            
            case COMMA:
                ch_state(q_state, curr_state, next, curr, line_num);
                break;

            case DOUBLE_QUOTE:
                ch_state(q_state, curr_state, next, curr, line_num);

                if (is_equation && curr_state != VALUE)
                    curr_state = VALUE;

                break;

            case NEWLINE:
                is_raw = false;
                is_equation = false;

                ch_state(q_state, curr_state, next, curr, line_num);
                line_num++;

                break;

            case AT:
                is_equation = true;
                ch_state(q_state, curr_state, next, curr, line_num);

                break;
            
            case POUND:
                is_raw = true;        
                ch_state(q_state, curr_state, next, curr, line_num);

                break;

            case COMMENT:
                token += curr;

                if (next == '\n')
                    push_state_clear_token(q_state, curr_state, token, next, line_num);

                break;
            
            case PERCENT:
                is_raw = true;        
                ch_state(q_state, curr_state, next, curr, line_num);

                break;

            case C_CODE:
                token += curr;

                if (next == '\n')
                    push_state_clear_token(q_state, curr_state, token, next, line_num);

                break;

            default:
                ch_state(q_state, curr_state, next, curr, line_num);
                break;
        }
    }
}

std::string EpicsTempFileLexAnalysis::prep_r_str(std::string r_str)
{
    remove_all_char(r_str, '\t');
    remove_all_char(r_str, '\r');

    // Not the most optimal solution - however this prevents memory leaks and unconditional branching from 
    // occurring though!
    for (size_t i = 0; i < r_str.length(); i++)
    {
        f_str += r_str[i];
    }

    return f_str;

void EpicsTempFileLexAnalysis::parse_dft(void)
{

}