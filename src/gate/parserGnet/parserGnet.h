/**
 * \brief Implements translation from Liberty format file to GNet.
 * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
 */

#pragma once

#include "gate/model/gnet.h"

#include <kernel/yosys.h>

#include <map>
#include <memory>
#include <vector>

struct NetData {
    std::vector<std::unique_ptr<eda::gate::model::GNet>> combNets;
    std::vector<std::unique_ptr<eda::gate::model::GNet>> memNets;
};

bool translateModuleToGNet(
    const Yosys::RTLIL::Module *m,
    eda::gate::model::GNet &net);

void translateDesignToGNet(
    const Yosys::RTLIL::Design *des,
    NetData &vec);

void translateLibertyToDesign(
    const std::string namefile,
    NetData &vec);

std::vector<uint64_t> buildTruthTab(
    const eda::gate::model::GNet *net);
