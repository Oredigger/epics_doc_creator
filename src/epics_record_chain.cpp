#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>
#include "epics_record_chain.hpp"
#include <iostream>

int EpicsRecordChain::load_rec_vert(q_token q_state)
{
    std::string rec_name, rec_type;
    Link_Type lt = NO_LINK;

    bool in_rec_header = false, in_rec_body = false;
    int  vert_num = 0;

    while (!q_state.empty())
    {
        auto elem = q_state.front();

        lex_states curr_state = std::get<0>(elem);
        std::string name = std::get<1>(elem);

        if (curr_state == HEADER && name == "record")
        {
            in_rec_header = true;
        }
        else if (in_rec_header)
        {
            if (curr_state == VALUE)
            {
                rec_name = name;
            }
            else if (curr_state == TYPE)
            {
                rec_type = name;
            }
            else if (curr_state == RIGHT_PAREN)
            {
                rec_vert[rec_name] = vert_num++;
                in_rec_header = false;
            }
        }
        else if (curr_state == LEFT_CURLY)
        {
            in_rec_body = true;
        }
        else if (in_rec_body)
        {
            if (curr_state == RIGHT_CURLY)
            {
                in_rec_body = false;
                rec_types[rec_name] = rec_type;
            }
            else if (curr_state == TYPE)
            {
                if (name == "FLNK")
                    lt = FLNK;
                else if (name.substr(0, 3) == "LNK" || name.substr(0, 3) == "OUT")
                    lt = OUT;
                else if (name.substr(0, 3) == "INP" || name.substr(0, 3) == "DOL")
                    lt = INP;
                else
                    lt = NO_LINK;
            }
            else if (curr_state == VALUE)
            {
                if (lt != NO_LINK)
                {
                    if (rec_type == "fanout" || lt == FLNK)
                    {
                        rec_links[rec_name].push(std::make_tuple(name, FLNK));

                        if (rec_vert.find(name) == rec_vert.end())
                            rec_vert[name] = vert_num++;
                    }
                    else
                    {
                        std::string delim = " ";
                        std::string token, name_copy;

                        bool is_pp = false;
                        size_t pos = 0, i = 0;
                        
                        while ((pos = name.find(delim)) != std::string::npos) 
                        {
                            token = name.substr(0, pos);
                            name.erase(0, pos + delim.length());

                            if (i++ == 0)
                                name_copy = token;

                            if (token == "PP")
                            {
                                is_pp = true;
                                break;
                            }
                        }
                        
                        if (is_pp || name.substr(0, 2) == "PP")
                        {
                            rec_links[rec_name].push(std::make_tuple(name_copy, lt));
                            lt = NO_LINK;

                            if (rec_vert.find(name_copy) == rec_vert.end())
                                rec_vert[name_copy] = vert_num++;
                        }
                    }      
                }
            }
        }

        q_state.pop();
    }

    return vert_num;
}

static bool is_all_zeros(std::vector<std::vector<int>> mat)
{
    for (auto row : mat) 
    {
        for (auto elem : row) 
        {
            if (elem) return false;
        }
    }

    return true;
}

static std::vector<std::vector<int>> init_adj_mat(int vert_num)
{
    std::vector<int> one_d_vec(vert_num, 0);
    std::vector<std::vector<int>> adj_mat;

    for (int i = 0; i < vert_num; i++)
    {
        adj_mat.push_back(one_d_vec);
    }

    return adj_mat;
}

bool EpicsRecordChain::is_start_chain(std::string vert_name)
{
    int vert_num = rec_vert[vert_name];

    for (auto row : adj_mat)
    {
        if (row[vert_num])
            return false;
    }

    return true;
}

void EpicsRecordChain::print_adj_mat(void)
{     
    std::cout << "Legend...\n";

    for (const auto& v : rec_vert)
    {
        std::cout << "Name: " << v.first << "    Vertex number: " << v.second  << "\n";
    }

    for (auto row : adj_mat) 
    {
        for (auto elem : row) 
        {
            std::cout << elem << " ";
        }

        std::cout << std::endl;
    }
}

void EpicsRecordChain::create_visual_graph(std::string fn)
{
    std::ofstream fout(fn);
    const size_t NEWLINE_SHAPE = 16;

    if (fout.good())
    {
        fout << "digraph G {\nrankdir=LR;\n";

        for (auto it = rec_vert.begin(); it != rec_vert.end(); it++)
        {
            std::string shape_name = it->first;
            std::string rec_type = rec_types[shape_name];

            if (shape_name.length() > NEWLINE_SHAPE)
            {
                shape_name.insert(NEWLINE_SHAPE, 1, '\\');
                shape_name.insert(NEWLINE_SHAPE + 1, 1, 'n');
            }

            fout << it->second << "[label=\"" << rec_type << "\n" << shape_name << "\" shape=box];\n";
        }

        int i = 0;

        for (i = 0; i < adj_mat.size(); i++) 
        {
            for (int j = 0; j < adj_mat[i].size(); j++) 
            {
                if (adj_mat[i][j])
                {
                    fout << i << "->" << j;

                    if (adj_mat[i][j] == OUT)
                        fout << "[ style=dotted color=red label=PP ]";
                    else if (adj_mat[i][j] == INP)
                        fout << "[ style=dotted color=blue label=PP ]";

                    fout << ";\n";
                }
            }
        }

        fout << "}\n";
        fout.close();
    }
}

EpicsRecordChain::EpicsRecordChain(void)
{}

EpicsRecordChain::EpicsRecordChain(q_token q_state)
{
    n = load_rec_vert(q_state);
    adj_mat = init_adj_mat(n);

    for (const auto& r : rec_links)
    {
        std::string start = r.first;

        if (rec_vert.find(start) == rec_vert.end())
            continue;
        
        int v1 = rec_vert.at(start);
        std::queue<std::tuple<std::string, Link_Type>> temp = r.second;

        while (!temp.empty())
        {
            std::string rec_dest_name = std::get<0>(temp.front());
            Link_Type lt = std::get<1>(temp.front());

            if (rec_vert.find(rec_dest_name) != rec_vert.end())
            {
                int v2 = rec_vert.at(rec_dest_name);
                adj_mat[v1][v2] = lt;
            }

            temp.pop();
        }
    }
}