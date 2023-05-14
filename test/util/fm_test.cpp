//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet_test.h"
#include "gate/transformer/hmetis.h"
#include "util/fm.h"
#include "util/partition_hgraph.h"

#include "gtest/gtest.h"

#include <filesystem>

namespace fs = std::filesystem;

using eda::gate::model::makeRand;

struct FMAlgoConfig {
  int seed;
  int passes;
  int weightLimit;
  int nodeNumber;
  int edgeNumber;
  int edgeSizeLimit;
  double r;
  size_t step;
};


//  Tests FM hypergraph partitioning algorithm.
int testFM(int passes, double r, HyperGraph &hgraph,
            const std::string &outPath) {

  FMAlgo algo(hgraph.getEptr(), hgraph.getEind(), hgraph.getWeights(), r,
              passes);

  algo.fm();
  if(hgraph.getWeights().size() <= 100) {
    hgraph.print(algo.getSides());
  }
  hgraph.printArea(algo.getSides());
  hgraph.graphOutput(outPath, algo.getSides());
  return hgraph.countCutSet(algo.getDistrib());
}

void testRandom(const FMAlgoConfig &config, const std::string &configPath,
                const std::string &outSubPath) {

  const fs::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::string outPath = homePath / outSubPath;
  HyperGraph graph(config.nodeNumber, config.seed);

  graph.setRndWeights(config.weightLimit);
  graph.setRndEdges(config.edgeNumber, config.edgeSizeLimit);

  std::ofstream fout(outPath);
  fout.close();

  int cutsetFm = testFM(config.passes, config.r, graph, outPath);
  //int cutsetK = testKahypar(config.r, graph, configPath, outPath);

  std::cout << "FM cutset : " << cutsetFm << '\n';
  //std::cout << "Kahypar cutset : " << cutsetK << std::endl;
}

//  Tests fm algorithm with the hypergraph with pattern-created edges.
void testLinked(const FMAlgoConfig &config, const std::string &configPath,
                const std::string &outSubPath) {
  const fs::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::string outPath = homePath / outSubPath;
  HyperGraph graph(config.nodeNumber, config.seed);

  std::ofstream fout(outPath);
  fout.close();

  graph.setRndWeights(config.weightLimit);
  graph.addLinkedEdges(config.step);

  int cutsetFm = testFM(config.passes, config.r, graph, outPath);
  //int cutsetK = testKahypar(config.r, graph, configPath, outPath);

  std::cout << "FM cutset : " << cutsetFm << '\n';
  //std::cout << "Kahypar cutset : " << cutsetK << std::endl;
}

//  Tests fm algorithm with the hypergraph from input file.
int testInput(int passes, double r, const std::string &configPath,
               const std::string &inSubPath, const std::string &outSubPath) {
  const fs::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::string inPath = homePath / inSubPath;
  std::ifstream fin(inPath);
  int cutset = -1;

  if (fin.is_open()) {
    HyperGraph hgraph(fin);

    fin.close();

    const std::string outPath = homePath / outSubPath;
    std::ofstream fout(outPath);

    fout.close();

    int cutsetFm = testFM(passes, r, hgraph, outPath);
    //int cutsetK = testKahypar(r, hgraph, configPath, outPath);

    std::cout << "FM cutset : " << cutsetFm << '\n';
    //std::cout << "Kahypar cutset : " << cutsetK << std::endl;
    cutset = cutsetFm;
  } else {
    std::cerr << "Failed to open file " << inPath << '\n';
  }
  return cutset;
}

void testGate(const eda::gate::model::GNet &net, int passes, double r,
              const std::string &configPath, const std::string &outSubPath) {
  const fs::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::string outPath = homePath / outSubPath;
  std::ofstream fout(outPath);
  fout.close();

  HMetisPrinter formatter(net);
  HyperGraph hgraph(formatter.getWeights(), formatter.getEptr(),
                    formatter.getEind());

  int cutsetFm = testFM(passes, r, hgraph, outPath);
  //int cutsetK = testKahypar(r, hgraph, configPath, outPath);

  std::cout << "FM cutset : " << cutsetFm << '\n';
  //std::cout << "Kahypar cutset : " << cutsetK << std::endl;
}

TEST(FMTest, BookPartitionTest) {
  const std::string pathIn = "test/data/fm/test_Kahng_in.txt";
  const std::string pathOut = "test/data/fm/test_Kahng_out1.txt";
  const std::string pathOut2 = "test/data/fm/test_Kahng_out2.txt";

  EXPECT_EQ(testInput(1, 0.375, "", pathIn, pathOut), 2);
  EXPECT_EQ(testInput(2, 0.375, "", pathIn, pathOut2), 1);
}

TEST(FMTest, GatePartitionTest) {
  auto net = makeRand(1024, 256);
  std::cout<<"NET GENERATED\n";
  const std::string pathOut = "test/data/fm/test_gate_out.txt";

  testGate(*net.get(), 1000, 0.375, "", pathOut);
}

TEST(FMTest, RandPartitionTest) {
  FMAlgoConfig config;
  config.seed = 123;
  config.passes = 10000;
  config.weightLimit = 100;
  config.nodeNumber = 250;
  config.edgeNumber = 250;
  config.edgeSizeLimit = 10;
  config.r = 0.375;
  const std::string pathOut = "test/data/fm/graph_rand_250.txt";

  testRandom(config, "config_path", pathOut);
}

TEST(FMTest, StructurePartitionGraphTest) {
  FMAlgoConfig config;
  config.seed = 123;
  config.passes = 10000;
  config.weightLimit = 100;
  config.nodeNumber = 250;
  config.step = 30;
  config.r = 0.375;
  const std::string pathOut = "test/data/fm/graph_link_250.txt";

  testLinked(config, "config_path", pathOut);
}

TEST(FMTest, BigPartitionTest) {
  FMAlgoConfig config;
  config.seed = 123;
  config.passes = 10000;
  config.weightLimit = 100;
  config.nodeNumber = 100'000;
  config.step = 30;
  config.r = 0.375;
  const std::string out = "test/data/fm/graph_link_100000.txt";

  testLinked(config, "config_path", out);
}

