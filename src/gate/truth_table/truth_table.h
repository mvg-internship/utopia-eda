/**
 * \brief Implements translation from Liberty format file to GNet.
 * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
 */

#pragma once

#include "gate/model/gnet.h"
#include "gate/simulator/simulator.h"

#include <vector>

std::vector<uint64_t> buildTruthTab(
    const eda::gate::model::GNet *net);
