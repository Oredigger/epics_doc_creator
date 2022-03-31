#ifndef EPICS_DBD_PARSE_H
#define EPICS_DBD_PARSE_H

#include <string>
#include <tuple>
#include <vector>

struct DBD_menu
{
    std::string menu_name;
    std::vector<std::tuple<std::string, std::string>> choice;
};

struct DBD_fields
{
    std::string field_name;
    std::vector<std::tuple<std::string, std::string>> prop;
    DBD_menu menu;
};

struct DBD_recordtype
{
    std::string recordtype_name;
    std::vector<DBD_fields> field;
};

class EPICS_DBD_parse
{
    private:
        DBD_fields fields;
        DBD_menu menu;
        DBD_recordtype rec;
    public:
        EPICS_DBD_parse(void);
        EPICS_DBD_parse(std::string fn);
};

#endif