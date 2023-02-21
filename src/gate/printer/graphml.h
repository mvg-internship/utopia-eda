#include "gate/model/gnet.h"

#include <ostream>
#include <set>
#include <string>
#include <vector>

std::string link_deskriptor(Link link);
bool link_not_draw(std::set<std::string> &don, Link const &link);
std::ostream &operator<<(const GNet &model, std::ostream &output);