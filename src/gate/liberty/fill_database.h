/**
 * \brief Implements translation from Liberty format file to GNet.
 * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
 */

#pragma once

#include "gate/optimizer/rwdatabase.h"
#include "parser_liberty.h"

void fillDatabase(
    NetData &nets,
    eda::gate::optimizer::SQLiteRWDatabase &database);
