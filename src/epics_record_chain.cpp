#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>
#include "epics_record_chain.hpp"
#include <iostream>

int EpicsRecordChain::load_rec_vert(q_token q_state)
{
    std::string rec_name;
    lex_states prev_state = INVALID;

    bool in_rec_header = false, in_rec_body = false, is_rec_link = false;
    int  vert_num = 0;

    while (!q_state.empty())
    {
        auto elem = q_state.front();

        lex_states curr_state = std::get<0>(elem);
        std::string name = std::get<1>(elem);

        if (curr_state == HEADER && name == "record")
        {
            rec_name.clear();
            in_rec_header = true;
        }
        else if (in_rec_header)
        {
            if (curr_state == VALUE)
            {
                rec_name = name;
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
            }
            else if (curr_state == TYPE)
            {
                // Add compatibility for INP and OUT PP NPP flags
                if (name == "FLNK")
                {
                    is_rec_link = true;
                }
                else if (name.substr(0, 3) == "LNK")
                {
                    std::cout << "I love you\n";
                    is_rec_link = true;
                }
            }
            else if (curr_state == VALUE)
            {
                if (is_rec_link)
                {
                    rec_links[rec_name].push(name);
                    is_rec_link = false;
                }
            }
        }

        q_state.pop();
        prev_state = curr_state;
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

    if (fout.good())
    {
        fout << "digraph G {\n";

        for (auto it = rec_vert.begin(); it != rec_vert.end(); it++)
        {
            fout << it->second << "[label=" << it->first << "];\n";
        }

        for (int i = 0; i < adj_mat.size(); i++) 
        {
            for (int j = 0; j < adj_mat[i].size(); j++) 
            {
                if (adj_mat[i][j])
                    fout << i << "->" << j << " ;\n";
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
        std::queue<std::string> temp = r.second;

        while (!temp.empty())
        {
            std::string dest = temp.front();

            if (rec_vert.find(dest) != rec_vert.end())
            {
                int v2 = rec_vert.at(dest);
                adj_mat[v1][v2] = 1;
            }

            temp.pop();
        }
    }
}