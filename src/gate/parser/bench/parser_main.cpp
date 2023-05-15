//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <parser.h>

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    try {
        std::cout << *parseBenchFile(argv[i]);
    } catch (std::exception& e) {
        std::cerr <<  std::endl; 
    }
  }
  return 0;
}
