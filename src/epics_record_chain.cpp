#include <iostream>
#include "epics_record_chain.hpp"

size_t EpicsRecordChain::load_rec_vert(q_token &q_state)
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
                rec_name = name;

            if (curr_state == RIGHT_PAREN)
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
                if (name == "FLNK" || name.substr(0, 3) == "LNK")
                    is_rec_link = true;
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

static void trav_fwd(std::vector<int> chain, std::vector<std::vector<int>> &mat, int vertex_num)
{
    if (vertex_num > mat.size() - 1)
        return;
    
    chain.push_back(vertex_num);
    int i = 0;

    for (i; i < adj_mat[vertex_num].size(); i++)
    {
        if (adj_mat[vertex_num][i])
        {
            adj_mat[vertex_num][i] = 0;
            break;
        }
    }

    trav_fwd(chain, mat, i);
}

static void trav_back(std::vector<int> chain, std::vector<std::vector<int>> &mat, int vertex_num)
{
    if (vertex_num < 0)
        return;
    
    chain.push(vertex_num);
    int i = 0

    for (i; i < adj_mat[vertex_num].size(); i++)
    {
        if (adj_mat[vertex_num][i])
        {
            adj_mat[vertex_num][i] = 0;
            break;
        }
    }

    trav_back(chain, mat, i);
}


void EpicsRecordChain::find_paths(void)
{
    if (adj_mat.empty())
        return;

    std::vector<std::vector<int>> ref_adj_mat = adj_mat;
    int curr_chain_num = 1;

    while (!is_all_zeros(ref_adj_mat))
    {
        int i = 0, j = 0;
        bool quit_loop = false;

        for (i = 0; i < adj_mat.size(); i++)
        {
            if (quit_loop)
                break;

            for (j = 0; j < adj_mat[i].size(); j++)
            {
                if (adj_mat[i][j])
                {
                    quit_loop = true;
                    break;
                }
            }
        }

        // Look backwards
        rec_chain_paths["CHAIN_" + std::to_string(curr_chain_num)];
    }
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

EpicsRecordChain::EpicsRecordChain(void)
{};

EpicsRecordChain::EpicsRecordChain(q_token &q_state)
{
    size_t vert_num = load_rec_vert(q_state);
    std::vector<int> one_d_vec(vert_num, 0);

    for (int i = 0; i < vert_num; i++)
    {
        adj_mat.push_back(one_d_vec);
    }

    for (const auto& r : rec_links)
    {
        std::string start = r.first;

        if (rec_vert.find(start) == rec_vert.end())
            continue;
        
        int vert_1 = rec_vert.at(start);
        std::queue<std::string> temp = r.second;

        while (!temp.empty())
        {
            std::string dest = temp.front();

            if (rec_vert.find(dest) != rec_vert.end())
            {
                int vert_2 = rec_vert.at(dest);
                adj_mat[vert_1][vert_2] = 1;
            }

            temp.pop();
        }
    }
};