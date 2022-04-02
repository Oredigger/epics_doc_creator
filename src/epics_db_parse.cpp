#include <algorithm>
#include <cstring>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

#include "epics_db_parse.hpp"
#include "parse_util.hpp"

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
        case AT:
            return "AT";
        case POUND:
            return "POUND";
        case COMMENT:
            return "COMMENT";
        case START:
            return "START";
    }

    return "INVALID";
}

static const std::string math_op = "()e-+*/%^><=&|!~?:., ";

static bool is_math_op(char next)
{
    for (auto c : math_op)
    {
        if (next == c)
            return true;
    }

    return false;
}

static void next_state(dpl_states &state, char next)
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
        default:
            if (state != VALUE && state != COMMENT) state = INVALID;
            break;
    }
}

static void ch_state(q_token &q_state, dpl_states &state, char next, char curr, size_t line_num)
{
    q_state.push(std::make_tuple(state, std::string(1, curr), line_num));

    if (state == POUND)
    {
        state = (next == '\n') ? NEWLINE : COMMENT;
        return;
    }
    
    if (state == AT)
    {
        if (isalpha(next) || isdigit(next) || next == '_')
            state = VALUE;
        else
            next_state(state, next);
        
        return;
    }

    if (isalpha(next) || next == '_')
        state = (curr == '(') ? TYPE : (curr == ',' || curr == '"') ? VALUE : HEADER;
    else if (isdigit(next) || next == '.')
        state = VALUE;
    else
        next_state(state, next);
}

static void push_state_clear_token(q_token &q_state, dpl_states &state, std::string &token, char next, size_t line_num)
{
    auto prev_state = state;
    next_state(state, next);

    if (prev_state != state)
    {
        q_state.push(std::make_tuple(prev_state, token, line_num));
        token.clear();
    }
}

static q_token parse_dft(std::string r_str)
{
    remove_all_char(r_str, '\t');
    remove_all_char(r_str, '\r');

    // Do not remove spaces in substrings that are surrounded by quotes.
    std::queue<size_t> q_loc = get_all_char_pos(r_str, '"');
    std::string f_str;

    size_t q_idx_0 = 0, q_idx_1 = 0;

    // Not the most optimal solution - however this prevents memory leaks and unconditional branching from 
    // occurring though!
    for (size_t i = 0; i < r_str.length(); i++)
    {
        if (i == q_idx_1)
        {
            if (q_loc.empty())
            {
                q_idx_1 = r_str.length() - 1;
            }
            else
            {
                q_idx_0 = q_loc.front();
                q_loc.pop();

                q_idx_1 = q_loc.front();
                q_loc.pop();
            }
        }

        if ((i <= q_idx_0 || i >= q_idx_1) && r_str[i] != ' ')
            f_str += r_str[i];
        else if (i > q_idx_0 && i < q_idx_1)
            f_str += r_str[i];
    }

    q_token q_state;

    if (f_str.empty())
        return q_state;

    dpl_states curr_state;

    if (isalpha(f_str[0]) || isdigit(f_str[0]) || f_str[0] == '_')
        curr_state = HEADER;
    else    
        next_state(curr_state, f_str[0]);
    
    bool is_equation = false, is_comment = false;
    size_t line_num = 1;
    
    std::string token;
    f_str += ' ';
    std::cout << f_str.length() << std::endl;

    for (size_t i = 0; i < f_str.length() - 1; i++)
    {
        char curr = f_str[i];
        char next = f_str[i + 1];

        switch (curr_state)
        {
            case HEADER:
                token += curr;

                if (!isalpha(next) && !isdigit(next) 
                    && next != '_' && next != ':' && !is_comment)
                    push_state_clear_token(q_state, curr_state, token, next, line_num);
                
                break;
            
            case TYPE:
                token += curr;
                
                if (!isalpha(next) && !isdigit(next) && next != '_' && !is_comment)
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
                        && next != 'e' && next != 'E' && !is_comment)
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
                is_comment = false;
                is_equation = false;

                ch_state(q_state, curr_state, next, curr, line_num);
                line_num++;

                break;

            case AT:
                is_equation = true;
                ch_state(q_state, curr_state, next, curr, line_num);

                break;
            
            case POUND:
                is_comment = true;        
                ch_state(q_state, curr_state, next, curr, line_num);

                break;

            case COMMENT:
                token += curr;

                if (next == '\n')
                    push_state_clear_token(q_state, curr_state, token, next, line_num);

                break;
        }
    }

    return q_state;
}

EPICS_DB_parse::EPICS_DB_parse(void)
{
    q_state = q_token();
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
        {
            os << fin.rdbuf() << std::endl;
            r_str = os.str();
        }
    }

    fin.close();
    q_state = parse_dft(r_str);
}

void EPICS_DB_parse::print_q_state(void)
{
    q_token q_copy = q_state;

    while (!q_copy.empty())
    {
        auto elem = q_copy.front();
        std::string token = (std::get<0>(elem) == NEWLINE) ? 
                            "\\n" : std::get<1>(elem);

        std::cout << state_2_str(std::get<0>(elem)) << "  " << token << "  " << std::get<2>(elem) << std::endl;
        
        if (std::get<0>(elem) == NEWLINE)
            std::cout << "\n";

        q_copy.pop();
    }
}

q_token EPICS_DB_parse::get_q_state(void)
{
    return q_state;
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