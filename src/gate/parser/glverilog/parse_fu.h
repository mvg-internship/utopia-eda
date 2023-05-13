#pragma once
#include <gate/model/gate.h>
#include <string>
#include <vector>
#include <memory>


bool parseGateLevelVerilog(const std::string &path, 
                           std::vector<std::unique_ptr<eda::gate::model::GNet>> &nets);
                           