/**
 * \brief Implements translation from Liberty format file to GNet.
 * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
 */

#pragma once

#include "gate/model/gnet.h"

#include <kernel/yosys.h>
#include <vector>

void translateModuleToGNet(
    const std::pair<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*> &m,
    eda::gate::model::GNet &net);

void translateDesignToGNet(
    const Yosys::RTLIL::Design &des,
    std::vector<eda::gate::model::GNet> &vec);


