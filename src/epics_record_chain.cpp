#include <iostream>
#include "epics_record_chain.hpp"

size_t EpicsRecordChain::load_rec_vert(q_token &q_state)
{
    std::string rec_name;

    bool in_rec_header = false, in_rec_body = false, is_rec_link = false;
    int  vert_num = 0;

    while (!q_state.empty())
    {
        auto elem = q_state.front();

        lex_states state = std::get<0>(elem);
        std::string name = std::get<1>(elem);

        if (state == HEADER && name == "record")
        {
            in_rec_header = true;
        }
        else if (in_rec_header)
        {
            if (state == VALUE)
                rec_name = name;
            if (state == RIGHT_PAREN)
                in_rec_header = false;
        }
        else if (state == LEFT_CURLY)
        {
            in_rec_body = true;
        }
        else if (in_rec_body)
        {
            if (state == RIGHT_CURLY)
            {
                rec_vert[rec_name] = vert_num++;
                in_rec_body = false;
                rec_name = "";
            }
            else if (state == TYPE)
            {
                // We probably have to include LNK from the fanout record.
                if (name == "FLNK" || name.substr(0, 3) == "LNK")
                    is_rec_link = true;
            }
            else if (state == VALUE)
            {
                if (is_rec_link)
                {
                    rec_links[rec_name].push(name);
                    is_rec_link = false;
                }
            }
        }

        q_state.pop();
    }

    return vert_num;
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
