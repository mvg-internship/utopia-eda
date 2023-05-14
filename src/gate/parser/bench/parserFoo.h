#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/model/gsymbol.h"
#include "tokens.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

std::unique_ptr<eda::gate::model::GNet> parseBenchFile(const std::string &filename);
