//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "graphml.h"

#include <string>

namespace eda::printer::graphMl {

const std::string toGraphMl::linkToString (const Link &link) {
  return std::to_string(link.source) + "_" + std::to_string(link.target) 
  + "_" + std::to_string(link.input);
}

void toGraphMl::printer (std::ostream &output, const GNet &model) {
  // Document header
  output << R"(<?xml version="1.0" encoding="UTF-8"?>
            <graphml xmlns="http://graphml.graphdrawing.org/xmlns"  
            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns 
            http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">
            )"
         << "<graph id=\"G"
         << model.id()
         << "\" edgedefault=\"directed\">\n";

  const auto &allGates = model.gates();
  for (auto *const gate: allGates) {
    // Output a description of the nodes of the graph
    output << "<node id=\""
            << gate->id()
            << "\"/>\n";
    const auto &allLinksFromGate = gate->links();
    for (const auto link: allLinksFromGate) {
      // Check whether this node is the beginning for the edge
      if (link.source==gate->id()) {
        const std::string link_description =
            linkToString(link);
        output << "<edge id=\"l"
               << link_description
               << "\" source=\""
               << link.source
               << "\" target=\""
               << link.target
               << "\">\n"
               << "<data key=\"l_d"
               << link_description
               << "\">"
               << link.input
               << "</data>\n"
               << "</edge>\n";
        // If that isn't target node in the graph, 
        // then draw it and mark it in red
        if (!model.hasNode(link.target)) {
          output << "<node id=\""
                 << link.target
                 << "\">\n"
                 << "<data key=\"sv"
                 << link.target
                 << "\">red</data>\n"
                 << "</node>\n";
        }
      }
      else {
        // If that isn't source node in the graph, 
        // then draw it and mark it in green
        if (!model.hasNode(link.source)) {
          const std::string link_description =
            linkToString(link);
          output << "<node id=\""
                 << link.source
                 << "\">\n"
                 << "<data key=\"sv"
                 << link.source
                 << "\">green</data>\n"
                 << "</node>\n"
                 << "<edge id=\"l"
                 << link_description
                 << "\" source=\""
                 << link.source
                 << "\" target=\""
                 << link.target
                 << "\">\n"
                 << "<data key=\"l_d"
                 << link_description
                 << "\">"
                 << link.input
                 << "</data>\n"
                 << "</edge>\n";
        }
      }
    }
  }
  // End of document
  output << "</graph>\n"
         << "</graphml>";
  }

}; //namespace eda::printer::graphMl
