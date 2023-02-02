//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <filesystem>
#include <string>
#include <unordered_map>

#include <lorina/diagnostics.hpp>
#include <lorina/verilog.hpp>

#include "gate/parser/reader_gate.h"
#include "gate/transformer/hmetis.h"
#include "gtest/gtest.h"
#include "util/partition_hgraph.h"

using namespace lorina;

TEST(ParserVTest, all) {
  /*
  if (!getenv("UTOPIA_HOME")) {
    FAIL() << "UTOPIA_HOME is not set.";
  }*/

  const std::filesystem::path subCatalog = "test/data/gate/parser";
  const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::filesystem::path prefixPath = homePath / subCatalog;
  const std::filesystem::path prefixPathIn = prefixPath / "input";
  const std::filesystem::path prefixPathOut = prefixPath / "output";
  const std::string filenames = prefixPath / "verilog_filenames.txt";

  std::ifstream in(filenames);
  std::string infile;

  while (std::getline(in, infile)) {
    std::string filename = prefixPathIn / (infile + ".v");
    std::string outFilename = prefixPathOut / (infile + ".dot");
    std::string outBaseFilename = prefixPathOut / ("base" + infile + ".dot");

    text_diagnostics consumer;
    diagnostic_engine diag(&consumer);

    ReaderGate reader(infile);

    return_code result = read_verilog(filename, reader, &diag);
    EXPECT_EQ(result, return_code::success);
    reader.dotPrint(outFilename);
    //reader.print();

    HMetisPrinter metis(*reader.getGnet());
    HyperGraph graph(metis.getWeights(), metis.getEptr(),
                     metis.getEind());
    graph.graphOutput(outBaseFilename);
  }
  in.close();
}
