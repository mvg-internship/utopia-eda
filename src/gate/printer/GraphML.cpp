#include "gate/model/gnet.h"
#include <string>
#include <ostream>
#include <vector>
#include <set>

using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;

void to_GraphML(const GNet &model, std::ostream &output)
{
    output << R"HEADER(<?xml version="1.0" encoding="UTF-8"?>
    <graphml xmlns="http://graphml.graphdrawing.org/xmlns">
    )HEADER";
    output << "<graph id=\"";
    output << model.id();
    output << "\" edgedefault=\"directed\">\n";
    // Шапка документа 
    auto b = model.gates();
    std::set<std::string> don;
    for (long i = 0; i < b.size(); i++)
    {
        output << "<node id = \"";
        output << b[i]->id();
        output << "\"/>\n";
        // описываем вершины
        auto a = b[i]->links();
        for (long k = 0; k < a.size(); k++)
        {
            if(don.find(std::to_string(i) + "_"+std::to_string(k))==don.end() and don.find(std::to_string(k) + "_"+std::to_string(i))==don.end())//проверяем, что не рисовали это ребро
            {
            output << "<edge id = \"l" +std::to_string(i) + "_"+std::to_string(k) + "\" source = \"";
            output << a[k].source;
            output << "\" target = \"";
            output << a[k].target;
            output << "\">\n";
            output << "<data key = \"l_d" + std::to_string(i) + "_"+std::to_string(k) + "\">";
            output << a[k].input;
            output << "</data>\n";
            output << "</edge>\n";
            // описываем рёбра
            don.insert(std::to_string(i) + "_"+std::to_string(k));
            don.insert(std::to_string(k) + "_"+std::to_string(i));
            }
        }
    }
    output << "</graph>\n";
    output << "</graphml>";
}