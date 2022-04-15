#ifndef EPICS_LATEX_GEN_H
#define EPICS_LATEX_GEN_H

#include <map>
#include <string>
#include <tuple>
#include "epics_lex_analysis.hpp" 

static const std::string DOC_HEADER = "\\documentclass[12pt]{article}\n";
static const std::string FILE_BEGIN = "\\begin{document}\n";
static const std::string FILE_END   = "\\end{document}\n";

const std::string PARAM_KEYS[] = 
{
    "author",
    "brief", 
    "bug", 
    "desc",
    "file",
    "param",
    "remark",
}; 

const size_t PARAM_KEYS_LEN = 7;

const size_t AUTHOR_KEY = 0;
const size_t BRIEF_KEY  = 1;
const size_t BUG_KEY    = 2;
const size_t DESC_KEY   = 3;
const size_t FILE_KEY   = 4;
const size_t PARAM_KEY  = 5;
const size_t REMARK_KEY = 6;

std::string gen_header_str(q_token &q_state);

class EpicsLatexHeader
{
    public:
        EpicsLatexHeader(std::string h_str);
        EpicsLatexHeader(void);

        void load(std::string h_str);
        void clear(void);
        virtual std::string get_latex_str(void) = 0;

    protected:
        std::string conv;
        std::string author;
        std::string brief;
        std::string bug;
        std::string desc;
        std::string file;
        std::string param;
        std::string remark;
};

class EpicsLatexFileHeader : public EpicsLatexHeader
{
    public:
        EpicsLatexFileHeader(std::string h_str) : EpicsLatexHeader(h_str){};
        EpicsLatexFileHeader(void) : EpicsLatexHeader(){};

        std::string get_latex_str(void);
};

class EpicsLatexRecordHeader : public EpicsLatexHeader
{
    public:
        EpicsLatexRecordHeader(std::string h_str) : EpicsLatexHeader(h_str){};
        EpicsLatexRecordHeader(void) : EpicsLatexHeader(){};
        
        std::string get_latex_str(void);
};

class EpicsLatexRecordParam : public EpicsLatexHeader
{
    public:
        EpicsLatexRecordParam(std::string h_str) : EpicsLatexHeader(h_str){};
        EpicsLatexRecordParam(void) : EpicsLatexHeader(){};
        
        std::string get_latex_str(void);
};

class EpicsLatexRecordBody
{
    public:
        EpicsLatexRecordBody(q_token &q_state);
        EpicsLatexRecordBody(void);

        void load(q_token &q_state);
        void clear(void);
        std::string get_latex_str(void);
    private:
        std::string conv;
        std::string record_name;
        std::string record_type;
        std::map<std::string, std::string> field_value_m;
        std::map<std::string, std::string> field_comment_m;
};

class EpicsLatexGen
{
    public:
        // Eventually transform db_fn into a queue of strings of database filepaths.
        EpicsLatexGen(std::string db_fn, q_token q_state);
        
        void save_as_file(std::string tex_fn);
    private:
        std::string latex;
        EpicsLatexFileHeader   f_header;
        EpicsLatexRecordHeader r_header;
        EpicsLatexRecordBody   r_body;
};


#endif