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
    std::vector<eda::gate::model::GNet> combNets;
    std::vector<eda::gate::model::GNet> memNets;
};

void translateModuleToGNet(
    const Yosys::RTLIL::Module &m,
    NetData &net);

void translateDesignToGNet(
    const Yosys::RTLIL::Design &des,
    NetData &vec);

void translateLibertyToDesign(
    const char* namefile,
    NetData &vec);

std::vector<uint64_t> truthTab(
    std::unique_ptr<const eda::gate::model::GNet> net);
