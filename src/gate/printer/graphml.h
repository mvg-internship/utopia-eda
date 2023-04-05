#include "gate/model/gnet.h"

#include <ostream>
#include <set>
#include <string>
#include <vector>

using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;
using Link = Gate::Link;

namespace eda::printer::graphMl {

std::string linkDescription(const Link &link);
std::string linkDescriptionReverse(const Link &link);
bool linkDontDraw(std::set<std::string> &don, const Link &link);
std::ostream &operator<<(std::ostream &output, GNet &model);

} //namespace eda::printer::graphMl