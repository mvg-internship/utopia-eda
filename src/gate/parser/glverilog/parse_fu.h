#pragma once
#include <gate/model/gate.h>
#include <memory>
#include <string>
#include <vector>



bool parseGateLevelVerilog(const std::string &path, 
                           std::vector<std::unique_ptr<eda::gate::model::GNet>> &nets);
