#include "parserGnet.h"
#include <iostream>

int main(int argc, char* argv[]) {
  Yosys::yosys_setup();
  std::vector<eda::gate::model::GNet> vec;
  for (size_t o = 1; o < argc; ++o) {
    translateLibertyToDesign(argv[o], vec);
  }
  for (auto i : vec) {
    std::cout << i << "\n";
  }
  Yosys::yosys_shutdown();
}
