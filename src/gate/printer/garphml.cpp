#include "graphml.h"

std::string link_deskriptor(Link link)
{
	return std::to_string(link.source) + "_" + std::to_string(link.target);
}

bool link_not_draw(std::set<std::string> &links_draw, Link link)
{
	if (links_draw.find(link_deskriptor(link)) == links_draw.end() && links_draw.find(link_deskriptor(link)) == links_draw.end())
	{
		links_draw.insert(link_deskriptor(link));
		links_draw.insert(link_deskriptor(link));
		return true;
	}
	else
	{
		return false;
	}
}

std::ostream &operator<<(const GNet &model, std::ostream &output)
{
	output
			<< R"HEADER(<?xml version="1.0" encoding="UTF-8"?>
    <graphml xmlns="http://graphml.graphdrawing.org/xmlns">
    )HEADER"
			<< "<graph id=\""
			<< model.id()
			<< "\" edgedefault=\"directed\">\n";
	// Document header
	auto &all_gates = model.gates();
	std::set<std::string> links_draw;
	for (size_t number_of_gate = 0; number_of_gate < all_gates.size(); number_of_gate++)
	{
		output
				<< "<node id = \""
				<< all_gates[number_of_gate]->id()
				<< "\"/>\n";
		// describing the nodes
		auto &all_links_from_gate = all_gates[number_of_gate]->links();
		for (size_t number_of_link = 0; number_of_link < all_links_from_gate.size(); number_of_link++)
		{
			if (link_not_draw(links_draw, all_links_from_gate[number_of_link])) // we check that we did not draw this edge
			{
				std::string link_deskription = link_deskriptor(all_links_from_gate[number_of_link]);
				output
						<< "<edge id = \"l" + link_deskription + "\" source = \""
						<< all_links_from_gate[number_of_link].source
						<< "\" target = \""
						<< all_links_from_gate[number_of_link].target
						<< "\">\n"
						<< "<data key = \"l_d" + link_deskription + "\">"
						<< all_links_from_gate[number_of_link].input
						<< "</data>\n"
						<< "</edge>\n";
				// describing the edges
			}
		}
	}
	output
			<< "</graph>\n"
			<< "</graphml>";
}