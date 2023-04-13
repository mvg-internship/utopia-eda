//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "graphml.h"

#include <set>
#include <string>

using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;
using Link = Gate::Link;

namespace eda::printer::graphMl {

std::string linkDescription(const Link &link) {
  return std::to_string(link.source) + "_" + std::to_string(link.target) 
  + "_" + std::to_string(link.input);
}

std::string linkDescriptionReverse(const Link &link) {
  return std::to_string(link.target) + "_" + std::to_string(link.source) 
  + "_" + std::to_string(link.input);
}

bool linkDontDraw(std::set<std::string> &linksDraw, const Link &link) {
  if (linksDraw.find(linkDescription(link)) == linksDraw.end() &&
      linksDraw.find(linkDescriptionReverse(link)) == linksDraw.end()) {
    linksDraw.insert(linkDescription(link));
    return true;
  }
  return false;
}

std::ostream &operator<<(std::ostream &output, GNet &model) {
  // Document header
  output << 
         R"(<?xml version="1.0" encoding="UTF-8"?>
            <graphml xmlns="http://graphml.graphdrawing.org/xmlns"  
            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns 
            http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">
            )"
      << "<graph id=\"G"
      << model.id()
      << "\" edgedefault=\"directed\">\n";

  const auto &allGates = model.gates();
  std::set<std::string> linksDraw;
  std::set<size_t> gatesDraw;
  for (auto *const gate: allGates) {
    //Adding a description of the vertex if it has not been described yet
    if((gatesDraw.find(gate->id())) == 
        gatesDraw.end())
    {
      output << 
                "<node id=\""
             << gate->id()
             << "\"/>\n";
      gatesDraw.insert(gate->id());
    }
    //We describe all the edges belonging to the vertex, 
    //provided that they have not already been described
    auto &allLinksFromGate = gate->links();
    for (const auto link: allLinksFromGate) {
      if (linkDontDraw(linksDraw, link)) {
        //We describe the edge by talking about its source, destination 
        //and which input it comes to
        const std::string link_deskription =
            linkDescription(link);
        output <<
               "<edge id=\"l" + link_deskription + "\" source=\""
            << link.source
            << "\" target=\""
            << link.target
            << "\">\n"
            << "<data key=\"l_d" + link_deskription + "\">"
            << link.input
            << "</data>\n"
            << "</edge>\n";
        //We check whether there are vertices not drawn in the graph, 
        //but into which the edge comes or from which it comes
        if((gatesDraw.find(link.source)) == 
        gatesDraw.end()) {
          gatesDraw.insert(link.source);
          output << "<node id=\""
                 << link.source
                 << "\"/>\n";
        }
        if((gatesDraw.find(link.target)) == 
        gatesDraw.end()) {
          gatesDraw.insert(link.target);
          output << 
                  "<node id=\""
                 << link.target
                 << "\"/>\n";
        }
      }
    }
  }
  //End of document
  output << "</graph>\n"
         << "</graphml>";
  return output;
}

} //namespace eda::printer::graphMl