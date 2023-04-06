#include "graphml.h"

namespace eda::printer::graphMl {

std::string linkDescription(const Link &link) {
  return std::to_string(link.source) + "_" + std::to_string(link.target);
}

std::string linkDescriptionReverse(const Link &link) {
  return std::to_string(link.target) + "_" + std::to_string(link.source);
}

bool linkDontDraw(std::set<std::string> &linksDraw, const Link &link) {
  if (linksDraw.find(linkDescription(link)) == linksDraw.end() &&
      linksDraw.find(linkDescriptionReverse(link)) == linksDraw.end()) {
    linksDraw.insert(linkDescription(link));
    linksDraw.insert(linkDescriptionReverse(link));
    return true;
  }
  return false;
}

std::ostream &operator<<(std::ostream &output, GNet &model) {
  output
      << R"HEADER(<?xml version="1.0" encoding="UTF-8"?>
<graphml xmlns="http://graphml.graphdrawing.org/xmlns"  
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns 
     http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">
    )HEADER"
      << "<graph id=\"G"
      << model.id()
      << "\" edgedefault=\"directed\">\n";
  // Document header
  auto &allGates = model.gates();
  std::set<std::string> linksDraw;
  for (size_t numberOfGate = 0; numberOfGate < allGates.size();
       numberOfGate++) {
    output
        << "<node id=\"n"
        << allGates[numberOfGate]->id()
        << "\"/>\n";
    // describing the nodes
    auto &allLinksFromGate = allGates[numberOfGate]->links();
    for (size_t numberOfLink = 0;
         numberOfLink < allLinksFromGate.size(); numberOfLink++) {
      if (linkDontDraw(linksDraw, allLinksFromGate[numberOfLink])) {
      // we check that we did not draw this edge
        std::string link_deskription =
            linkDescription(allLinksFromGate[numberOfLink]);
        output
            << "<edge id=\"l" + link_deskription + "\" source=\""
            << allLinksFromGate[numberOfLink].source
            << "\" target=\""
            << allLinksFromGate[numberOfLink].target
            << "\">\n"
            << "<data key=\"l_d" + link_deskription + "\">"
            << allLinksFromGate[numberOfLink].input
            << "</data>\n"
            << "</edge>\n";
        // describing the edges
      }
    }
  }
  output
      << "</graph>\n"
      << "</graphml>";
  return output;
}

} //namespace eda::printer::graphMl