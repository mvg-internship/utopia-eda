#include "gate/model/gnet.h"

#include <ostream>
#include <set>
#include <string>
#include <vector>

using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;

std::ostream &operator<<(const GNet &model, std::ostream &output);