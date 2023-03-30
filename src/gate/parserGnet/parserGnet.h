/**
 * \brief Implements translation from Liberty format file to GNet.
 * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
 */

#include "base/model/signal.h"
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/simulator/simulator.h"
#include "passes/techmap/libparse.h"
#include <kernel/celltypes.h>
#include <kernel/yosys.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace gmodel = eda::gate::model;
namespace lib = Yosys::hashlib;
namespace yosys = Yosys::RTLIL;

using gate = gmodel::Gate;
using gsymbol = gmodel::GateSymbol;
using gid = gmodel::Gate::Id;

void printWires(
    const lib::dict<yosys::IdString, yosys::Wire*> &wires,
    gmodel::GNet &net,
    std::map<unsigned, gid> &inputs,
    std::map<unsigned, gid> &outputs);
gsymbol functionGateSymbol(const size_t type, std::map<int, std::string> &typeRTLIL, const int a);
gate::SignalList typeDffsr(
    const std::string type,
    const int node,
    const std::map<int, std::pair<int, int>> &cell,
    const std::map<unsigned, gid> &inputs);
gid buildNet(
    const int root,
    gmodel::GNet &net,
    const std::map<int, gsymbol> &typeFunc,
    const std::map<int, std::pair<int, int>> &cell,
    const std::map<unsigned, gid> &inputs,
    const std::map<int, std::string> &typeRTLIL);
void printCells(
    const lib::dict<yosys::IdString, yosys::Cell*> &cells,
    gmodel::GNet &net,
    std::map<int, gsymbol> &typeFunc,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &typeRTLIL);
void printConnections(
    const std::vector<std::pair<yosys::SigSpec, yosys::SigSpec>> &connections,
    gmodel::GNet &net,
    std::map<int, gsymbol> &typeFunc,
    std::map<int, std::pair<int, int>> &cell,
    std::map<unsigned, gid> &inputs,
    std::map<unsigned, gid> &outputs,
    std::map<int, std::string> typeRTLIL);
void translateModuleToGNet(
    const std::pair<yosys::IdString, yosys::Module*> &m,
    gmodel::GNet &net);
void translateDesignToGNet(const yosys::Design &des, std::vector<gmodel::GNet> &vec);


