#include "gate/model/gnet.h"

#include <ostream>
#include <set>
#include <string>
#include <vector>

using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;
using Link = Gate::Link;

std::string link_deskriptor(Link link);
bool link_not_draw(std::set<std::string> &don, Link link);
std::ostream &operator<<(const GNet &model, std::ostream &output);