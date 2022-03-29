#ifndef EPICS_DB_PARSE_H
#define EPICS_DB_PARSE_H

#include <map>
#include <string>
#include <queue>

struct DB_record_inst
{
    std::string recordinst_name;
    std::string recordtype_name;
    std::map<std::string, std::string> field;
};

class EPICS_DB_parse
{
    private:
        std::queue<DB_record_inst> q_inst;
    public:
        EPICS_DB_parse(void);
        EPICS_DB_parse(std::string fn);
};

short check_calc_eq(std::string pinfix);

#endif