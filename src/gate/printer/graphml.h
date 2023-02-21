#include "gate/model/gnet.h"

#include <ostream>
#include <set>
#include <string>
#include <vector>

std::string linkDescription(const Link &link);
bool linkDontDraw(std::set<std::string> &don, const Link &link);
std::ostream &operator<<(const GNet &model, std::ostream &output);