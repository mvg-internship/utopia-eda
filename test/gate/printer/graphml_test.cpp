#include "gate/printer/graphml.h"
#include "gate/model/gnet_test.h"

#include "gtest/gtest.h"

TEST(toGraphMlTest,all) {
  GNet test = *makeRand(100, 100);
  std::cout << test;
  //Выводит не то, что надо, так как оператор вывода для GNet уже перегружен, 
  //не совсем понимаю как выбрать нужный мне
}
