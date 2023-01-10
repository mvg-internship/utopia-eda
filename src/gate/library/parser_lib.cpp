#include "gate/library/parser_lib.h"
#include <kitty/kitty.hpp>

#include <fstream>
#include <vector>

namespace eda::gate::library {

void LibertyParser::readJson(std::string pathToLiberty) {
  std::ifstream file(pathToLiberty.c_str());
  jFile = json::parse(file);
}

void LibertyParser::conversionToTruthTable() {
  std::vector<std::string> inputPins;
  std::string outputVar;
  std::string function;

  for (const auto& [cellName, cellVal] : jFile.items()) {
    for (const auto& [pinDirect, pinVal] : cellVal.items()) {

      if (pinDirect == "input") {
        std::string inPinsName = pinVal;
        std::string token;
        std::stringstream ss(inPinsName);
        while (getline(ss, token, ' ')) {
          inputPins.push_back(token);
        }

      } else {
        for (const auto& [outPinName, outPinFunc] : pinVal.items()) {
          function = outPinFunc;
        }
      }
    }
    LibertyParser::writeCell(inputPins, cellName, function);

    inputPins.clear();
  }
}

void LibertyParser::writeCell(std::vector<std::string> inputPins,
    std::string cellName, std::string function) {
  cell = new kitty::dynamic_truth_table(inputPins.size());
  kitty::create_from_formula(*cell, function, inputPins);
  cells.push_back(*cell);
}

} // namespace eda::gate::library 