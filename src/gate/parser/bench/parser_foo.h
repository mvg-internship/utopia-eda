#include "gate/model/gnet.h"

#include <memory>

std::unique_ptr<eda::gate::model::GNet> parseBenchFile(const std::string &filename);
