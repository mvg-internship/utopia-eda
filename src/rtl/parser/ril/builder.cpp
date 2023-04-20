//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "rtl/parser/ril/builder.h"

#include <cassert>
#include <iostream>
#include <unordered_map>

using namespace eda::rtl;

namespace eda::rtl::parser::ril {

std::unique_ptr<Builder> Builder::_instance = nullptr;

Type Builder::to_type(const std::string &type) const {
  auto kind = (type.at(0) == 's' ? Type::SINT : Type::UINT);
  auto width = static_cast<std::size_t>(std::stoi(type.substr(2)));
  return Type(kind, width);
}

std::vector<bool> Builder::to_value(const std::string &value) const {
  std::vector<bool> result(value.size() - 2);
  for (std::size_t i = 2; i < value.size(); i++) {
    result[i - 2] = (value.at(i) != '0');
  }
  return result;
}

Variable Builder::to_var(const std::string &value) const {
  Type type(Type::UINT, value.size() - 2);
  Variable var("$" + value, Variable::WIRE, Variable::INNER, type);
  return var;
}

std::unique_ptr<Net> Builder::create() {
  auto net = std::make_unique<Net>();

  // Collect all declarations.
  std::unordered_map<std::string, Variable> variables;
  std::unordered_map<std::string, unsigned> def_count;

  for (const auto &decl: _model.decls) {
    auto v = variables.find(decl.name);
    if (v == variables.end()) {
      Variable var(decl.name, decl.kind, decl.bind, to_type(decl.type));
      variables.insert({ decl.name, var });
      def_count.insert({ decl.name, 0 });
    }
  }

  // Count variable definitions.
  for (const auto &proc: _model.procs) {
    for (const auto &assign: proc.action) {
      auto d = def_count.find(assign.out);
      if (d != def_count.end()) {
        d->second++;
      }
    }
  }

  // Create phi-nodes when required.
  std::unordered_map<std::string, VNode::Id> use_nodes;

  for (const auto &decl: _model.decls) {
    if (decl.bind == Variable::INPUT) {
      auto v = variables.find(decl.name);
      use_nodes[decl.name] = net->addSrc(v->second);
    }
  }

  for (const auto &proc: _model.procs) {
    for (const auto &assign: proc.action) {
      auto v = variables.find(assign.out);
      if (def_count[assign.out] > 1) {
        use_nodes[assign.out] = net->addPhi(v->second);
      } else if (v->second.kind() == Variable::WIRE) {
        use_nodes[assign.out] = net->addFun(v->second, assign.func);
      } else {
        use_nodes[assign.out] = net->addReg(v->second);
      }
    }
  }
 
  // Construct p-nodes.
  std::unordered_map<std::string, VNode::Id> val_nodes;

  for (const auto &proc: _model.procs) {
    VNode::Id triggerId = !proc.signal.empty() ?
        use_nodes[proc.signal] : VNode::INVALID;
    VNode::Signal event(proc.event, triggerId);

    VNode::List guard;
    VNode::List action;

    if (!proc.guard.empty()) {
      VNode::Id vnodeId = use_nodes[proc.guard];
      guard.push_back(VNode::get(vnodeId));
    }
 
    for (const auto &assign: proc.action) {
      VNode::SignalList inputs;

      for (const auto &in: assign.in) {
        if (in.at(0) == '0') {
	  VNode::Id valueId = VNode::INVALID;
          auto i = val_nodes.find(in);

	  if (i != val_nodes.end()) {
            valueId = i->second;
          } else {
            valueId = net->addVal(to_var(in), to_value(in));
	    val_nodes[in] = valueId;
          }

	  inputs.push_back(VNode::Signal::always(valueId));
          break;
        }

        auto i = use_nodes.find(in);
        assert(i != use_nodes.end());
        inputs.push_back(VNode::Signal::always(i->second));
      }

      VNode::Id vnodeId = VNode::INVALID;
      auto v = variables.find(assign.out);

      if (def_count[assign.out] == 1) {
        vnodeId = use_nodes[assign.out];
        net->update(vnodeId, inputs);
      } else if (v->second.kind() == Variable::WIRE) {
        vnodeId = net->addFun(v->second, assign.func, inputs);
      } else {
        assert(!inputs.empty());
	vnodeId = net->addReg(v->second, inputs.front());
      }

      action.push_back(VNode::get(vnodeId));
    }

    net->addSeq(event, guard, action);
  }

  return net;
}

} // namespace eda::rtl::parser::ril
