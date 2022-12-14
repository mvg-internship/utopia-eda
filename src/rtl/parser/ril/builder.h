//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "base/model/signal.h"
#include "rtl/model/fsymbol.h"
#include "rtl/model/net.h"
#include "rtl/model/variable.h"

using namespace eda::base::model;
using namespace eda::rtl::model;

namespace eda::rtl::parser::ril {

struct AstDecl final {
  AstDecl(Variable::Kind kind, Variable::Bind bind,
      const std::string &name, const std::string &type):
    kind(kind), bind(bind), name(name), type(type) {}

  AstDecl() = default;

  Variable::Kind kind;
  Variable::Bind bind;
  std::string name;
  std::string type;
};

struct AstAssign final {
  AstAssign(FuncSymbol func, const std::string &out, const std::vector<std::string> &in):
    func(func), out(out), in(in) {}

  AstAssign() = default;

  FuncSymbol func;
  std::string out;
  std::vector<std::string> in;
};

struct AstProc final {
  AstProc() = default;

  Event event;
  std::string signal;
  std::string guard;
  std::vector<AstAssign> action;
};

struct AstModel final {
  AstModel() = default;

  std::vector<AstDecl> decls;
  std::vector<AstProc> procs;
};

/**
 * \brief Helps to contruct the IR from source code.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class Builder final {
public:
  static Builder& get() {
    if (_instance == nullptr) {
      _instance = std::unique_ptr<Builder>(new Builder());
    }
    return *_instance;
  }

  std::vector<bool> to_value(const std::string &value) const;
  Variable to_var(const std::string &value) const;

  Type to_type(const std::string &type) const;

  std::unique_ptr<Net> create();

  void start_model() {
    _model.decls.clear();
    _model.procs.clear();
  }

  void add_decl(Variable::Kind kind, Variable::Bind bind,
      const std::string &name, const std::string &type) {
    _model.decls.push_back(AstDecl(kind, bind, name, type));
  }

  void start_proc() {
    _proc.event = Event::ALWAYS;
    _proc.signal.clear();
    _proc.guard.clear();
    _proc.action.clear();
  }

  void end_proc() {
    _model.procs.push_back(_proc);
  }

  void set_event(Event event, const std::string &signal) {
    _proc.event = event;
    _proc.signal = signal;
  }

  void set_guard(const std::string &guard) {
    _proc.guard = guard;
  }

  void add_assign(FuncSymbol func, const std::string &out, const std::vector<std::string> &in) {
    _proc.action.push_back(AstAssign(func, out, in));
  }

private:
  Builder() {}

  AstModel _model;
  AstProc _proc;

  static std::unique_ptr<Builder> _instance;
};

} // namespace eda::rtl::parser::ril
