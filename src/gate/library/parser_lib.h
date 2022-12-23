#pragma once

#include "kitty/kitty.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace eda::gate::library {

class LibertyParser {

private:
  json jFile;
  kitty::dynamic_truth_table *cell;

  void writeCell(std::vector<std::string> inputPins, 
      std::string cellName, std::string function);

public:
  std::vector <kitty::dynamic_truth_table> cells;
  void readJson(std::string aPathToLiberty);
  void conversionToTruthTable();
};

} // namespace eda::gate::library