#include "epics_db_parse.hpp"

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