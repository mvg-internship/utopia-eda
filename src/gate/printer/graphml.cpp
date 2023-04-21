//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "graphml.h"

namespace eda::printer::graphMl {

std::map <std::string, std::string> toGraphMl::colours = {
  {"red", "red"}, 
  {"green", "green"}, 
  {"black", "black"}
};

void toGraphMl::printNode(std::ostream &output, uint32_t nodeId,
 std::string colour) {
  output << 
    "<node id=\"" <<
    nodeId <<
    "\">\n" <<
    "<data key=\"sv" <<
    nodeId <<
    "\">" <<
    colours[colour] <<
    "</data>\n" <<
    "</node>\n";
}

void toGraphMl::printEdge(std::ostream &output, const Link &link) {
  const std::string link_description = linkToString(link);
  output << 
    "<edge id=\"l" <<
    link_description <<
    "\" source=\"" <<
    link.source <<
    "\" target=\"" <<
    link.target <<
    "\">\n" <<
    "<data key=\"l_d" <<
    link_description <<
    "\">" <<
    link.input <<
    "</data>\n" <<
    "</edge>\n";
}

const std::string toGraphMl::linkToString (const Link &link) {
  return std::to_string(link.source) + "_" + std::to_string(link.target) 
  + "_" + std::to_string(link.input);
}

void toGraphMl::printer (std::ostream &output, const GNet &model) {
  // Document header
  output <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" <<
    "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"\n" <<
    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n" <<
    "xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns\n" <<
    "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n" <<
    "<graph id=\"G" <<
    model.id() <<
    "\" edgedefault=\"directed\">\n";
  const auto &allGates = model.gates();
  for (auto *const gate: allGates) {
    // Output a description of the nodes of the graph and maks it black
    printNode(output, gate->id(), "black");
    const auto &allLinksFromGate = gate->links();
    for (const auto link: allLinksFromGate) {
      // Check whether this node is the beginning for the edge
      if (link.source == gate->id()) {
        printEdge(output, link);
        // If the target node isn't in the graph, 
        // then draw it and mark it red
        if (!model.hasNode(link.target)) {
          printNode(output, link.target, "red");
        }
      }
      else {
        // If the source node isn't in the graph, 
        // then draw it and mark it in green
        if (!model.hasNode(link.source)) {
          printNode(output, link.source, "green");
          printEdge(output, link);
        }
      }
    }
  }
  // End of document
  output << 
  "</graph>\n" <<
  "</graphml>";
}

}; //namespace eda::printer::graphMl
