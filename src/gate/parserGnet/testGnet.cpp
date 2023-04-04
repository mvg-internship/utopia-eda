#include "parserGnet.h"
#include <iostream>
int main(int argc, char* argv[]) {
  std::ostream& out = std::cout;
  Yosys::yosys_setup();
  std::vector<eda::gate::model::GNet> vec;
  for (size_t o = 1; o < argc; ++o) {
    Yosys::RTLIL::Design design;
    Yosys::run_frontend(argv[o], "liberty", &design, nullptr);
    translateDesignToGNet(design, vec);
  }
  for (auto i : vec) {
    std::cout << i << "\n";
  }
  Yosys::yosys_shutdown();
}
