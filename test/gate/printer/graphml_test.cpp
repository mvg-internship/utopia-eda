#include "gate/printer/graphml.h"
#include "gate/model/gnet_test.h"

#include "gtest/gtest.h"

using namespace eda::printer::graphMl;

TEST(toGraphMlTest,all) {
  GNet test = *makeRand(10000, 10000);
  std::cout << test;
}
