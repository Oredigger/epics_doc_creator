#include "epics_record_chain.hpp"
#include <fstream>
#include <iostream>

int EpicsRecordChain::load_rec_vert(q_token q_state)
{
    std::string rec_name, rec_type;
    Link_Type lt = NO_LINK;

    bool in_rec_header = false, in_rec_body = false, is_stream = false;
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
            switch (curr_state)
            {
                case VALUE:
                    rec_name = name;
                    break;
                case TYPE:
                    rec_type = name;
                    break;
                case RIGHT_PAREN:
                    rec_vert[rec_name] = vert_num++;
                    rec_types[rec_name] = rec_type;
                    in_rec_header = false;
                    break;
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
                is_stream = false;
            }
            else if (curr_state == TYPE)
            {
                if (name == "FLNK")
                    lt = FLNK;
                else if (name.substr(0, 3) == "LNK" || name.substr(0, 3) == "OUT")
                    lt = OUT;
                else if (name.substr(0, 2) == "IN" || name.substr(0, 3) == "DOL" || name.substr(0, 4) == "SELL")
                    lt = INP;
                else
                    lt = NO_LINK;
            }
            else if (curr_state == VALUE)
            {
                if (name == "stream")
                    is_stream = true;

                if (lt != NO_LINK)
                {
                    std::string pv = "NONE";
                    size_t per = name.find('.');

                    if (per != std::string::npos)
                    {
                        size_t space = name.find(' ');
                        pv = name.substr(per + 1, space - per);
                        name.erase(per, space - per);
                    }

                    if ((rec_type == "fanout" && lt != INP) || lt == FLNK)
                    {
                        rec_links[rec_name].push(std::make_tuple(name, FLNK, NO_PROP, pv));

                        if (rec_vert.find(name) == rec_vert.end())
                            rec_vert[name] = vert_num++;
                    }
                    else
                    {
                        std::string delim = " ";
                        std::string token, name_copy;
                        name += " ";

                        size_t prop = 0, pos = 0, i = 0;
                        
                        while ((pos = name.find(delim)) != std::string::npos) 
                        {
                            token = name.substr(0, pos);
                            name.erase(0, pos + delim.length());

                            if (i++ == 0)
                                name_copy = token;

                            if (token == "PP")
                                prop |= PP;
                            else if (token == "CA")
                                prop |= CA;
                            else if (token == "CPP")
                                prop |= CPP;
                            else if (token == "CP")
                                prop |= CP;;
                        }
                        
                        if (prop && !is_stream)
                        {
                            if (rec_vert.find(name_copy) == rec_vert.end())
                                rec_vert[name_copy] = vert_num++;

                            if (prop & CPP || prop & CP)
                                rec_links[name_copy].push(std::make_tuple(rec_name, lt, prop, pv));
                            else
                                rec_links[rec_name].push(std::make_tuple(name_copy, lt, prop, pv));
                            
                            lt = NO_LINK;
                        }
                    }      
                }
            }
        }

        q_state.pop();
    }

    return vert_num;
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
            std::cout << std::get<0>(elem) << "," << std::get<1>(elem) << "," << std::get<2>(elem) << " ";
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
            std::string shape_name = it->first, rec_type = rec_types[shape_name];
            std::string pv;

            if (rec_type.empty())
                rec_type = "OUTSIDE_DB";

            if (shape_name.length() > NEWLINE_SHAPE)
            {
                shape_name.insert(NEWLINE_SHAPE, 1, '\\');
                shape_name.insert(NEWLINE_SHAPE + 1, 1, 'n');
            }

            fout << it->second << "[label=\"" << rec_type << "\n" << shape_name << "\"";
            
            if (rec_type == "OUTSIDE_DB")
                fout << "shape=box, style=filled, fillcolor=bisque]\n";
            else
                fout << "shape=box]\n";
        }

        for (int i = 0; i < adj_mat.size(); i++) 
        {
            for (int j = 0; j < adj_mat[i].size(); j++) 
            {
                Link_Type lt = std::get<0>(adj_mat[i][j]);

                if (lt != NO_LINK)
                {
                    fout << i << "->" << j;

                    if (lt == OUT || lt == INP)
                    {
                        if (lt == OUT)
                            fout << "[ color=red ";
                        else if (lt == INP)
                            fout << "[ color=blue ";

                        std::vector<std::string> edge_label;
                        int lp = std::get<1>(adj_mat[i][j]);

                        if (lp & PP)
                            edge_label.push_back("PP");
                        if (lp & CA)
                            edge_label.push_back("CA");
                        if (lp & CPP)
                            edge_label.push_back("CPP");
                        if (lp & CP)
                            edge_label.push_back("CP");

                        std::string pv = std::get<2>(adj_mat[i][j]);
                    
                        if (!pv.empty() && pv != "NONE")
                            edge_label.push_back(pv);

                        if (!edge_label.empty())
                        {
                            fout << "label=\"";

                            for (auto label : edge_label)
                            {
                                fout << label << " ";
                            }

                            fout << "\"";
                        }

                        fout << "]";
                    }

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
    std::vector<std::tuple<Link_Type, int, std::string>> temp(n, std::make_tuple(NO_LINK, NO_PROP, "NONE"));

    for (int i = 0; i < n; i++)
    {
        adj_mat.push_back(temp);
    }

    for (const auto& r : rec_links)
    {
        std::string start = r.first;

        if (rec_vert.find(start) == rec_vert.end())
            continue;
        
        int v1 = rec_vert.at(start);
        std::queue<std::tuple<std::string, Link_Type, int, std::string>> temp = r.second;

        while (!temp.empty())
        {
            std::string rec_dest_name = std::get<0>(temp.front()), pv = std::get<3>(temp.front());
            Link_Type lt = std::get<1>(temp.front());
            int lp = std::get<2>(temp.front());

            if (rec_vert.find(rec_dest_name) != rec_vert.end())
            {
                int v2 = rec_vert.at(rec_dest_name);
                adj_mat[v1][v2] = std::make_tuple(lt, lp, pv);
            }

            temp.pop();
        }
    }
}