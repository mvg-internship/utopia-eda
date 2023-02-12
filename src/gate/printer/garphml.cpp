#include "graphml.h"

std::ostream &operator<<(const GNet &model, std::ostream &output)
{
	output << R"HEADER(<?xml version="1.0" encoding="UTF-8"?>
    <graphml xmlns="http://graphml.graphdrawing.org/xmlns">
    )HEADER";
	output << "<graph id=\"";
	output << model.id();
	output << "\" edgedefault=\"directed\">\n";
	// Document header
	auto b = model.gates();
	std::set<std::string> don;
	for (long i = 0; i < b.size(); i++)
	{
		output
				<< "<node id = \""
				<< b[i]->id()
				<< "\"/>\n";
		// описываем вершины
		auto a = b[i]->links();
		for (long k = 0; k < a.size(); k++)
		{
			if (don.find(std::to_string(i) + "_" + std::to_string(k)) == don.end() && don.find(std::to_string(k) + "_" + std::to_string(i)) == don.end()) // проверяем, что не рисовали это ребро
			{
				output
						<< "<edge id = \"l" + std::to_string(i) + "_" + std::to_string(k) + "\" source = \""
						<< a[k].source
						<< "\" target = \""
						<< a[k].target
						<< "\">\n"
						<< "<data key = \"l_d" + std::to_string(i) + "_" + std::to_string(k) + "\">"
						<< a[k].input
						<< "</data>\n"
						<< "</edge>\n";
				// описываем рёбра
				don.insert(std::to_string(i) + "_" + std::to_string(k));
				don.insert(std::to_string(k) + "_" + std::to_string(i));
			}
		}
	}
	output
			<< "</graph>\n"
			<< "</graphml>";
}