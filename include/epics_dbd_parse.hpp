#ifndef EPICS_DBD_PARSE_H
#define EPICS_DBD_PARSE_H

#include <string>
#include <tuple>
#include <vector>

struct DbdMenu
{
    std::string menu_name;
    std::vector<std::tuple<std::string, std::string>> choice;
};

struct DbdFields
{
    std::string field_name;
    std::vector<std::tuple<std::string, std::string>> prop;
    DbdMenu menu;
};

struct DbdRecordtype
{
    std::string recordtype_name;
    std::vector<DbdFields> field;
};

class EpicsDbdParse
{
    private:
        DbdMenu menu;
        DbdFields fields;
        DbdRecordtype rec;
    public:
        EpicsDbdParse(void);
        EpicsDbdParse(std::string fn);
};

#endif