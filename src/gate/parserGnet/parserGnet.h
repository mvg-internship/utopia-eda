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

void translateModuleToGNet(
    const std::pair<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*> &m,
    eda::gate::model::GNet &net);
void translateDesignToGNet(
    const Yosys::RTLIL::Design &des,
    std::vector<eda::gate::model::GNet> &vec);


