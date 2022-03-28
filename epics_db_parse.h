#ifndef EPICS_DB_PARSE_H
#define EPICS_DB_PARSE_H

#include "postfix.h"
#include <string>

class EPICS_DB_parse
{
    private:
        size_t num_record_insts;
    public:
        EPICS_DB_parse();
};

short check_calc_eq(std::string pinfix);

#endif