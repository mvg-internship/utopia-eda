//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "util/singleton.h"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

namespace eda::gate::optimizer {

class RWDatabase;

class RewriteManager final : public util::Singleton<RewriteManager> {
  friend class util::Singleton<RewriteManager>;
  static constexpr const char *DEFAULT = "abc";

public:
  /// Initializes the rewriting database for the given library.
  void initialize(const std::string &library = DEFAULT);

  /// Returns the database for the given library.
  const RWDatabase &getDatabase(const std::string &library = DEFAULT) const {
    const auto i = db.find(library);
    assert(i != db.end());
    return *i->second;
  }
  
private:
  std::unordered_map<std::string, std::shared_ptr<RWDatabase>> db;
};

} // namespace eda::gate::optimizer
