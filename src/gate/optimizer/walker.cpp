//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/walker.h"

namespace eda::gate::optimizer {

  using GNet = eda::gate::model::GNet;

  Walker::Walker(Walker::GNet *gNet, Visitor *visitor, CutStorage *cutStorage) :
          gNet(gNet), visitor(visitor), cutStorage(cutStorage) {}

  void Walker::walk(bool forward) {

    auto nodes = eda::utils::graph::topologicalSort(*gNet);
    if (!forward) {
      std::reverse(nodes.begin(), nodes.end());
    }

    for (auto &node: nodes) {
      switch (callVisitor(node)) {
        case FINISH_ALL:
          return;
        case FINISH_THIS:
          continue;
        case SUCCESS:
          break;
      }
    }
  }

  void Walker::walk(GateID start, const Cut &cut, bool forward) {
    std::queue<GateID> bfs;
    bfs.push(start);

    std::unordered_set<GateID> accessed;

    // First trace to define needed nodes.
    while (!bfs.empty()) {
      GateID cur = bfs.front();
      bfs.pop();
      accessed.emplace(cur);
      if (cut.find(cur) != cut.end()) {
        continue;
      }
      auto next = getNext(cur, forward);
      for (auto node: next) {
        bfs.push(node);
      }
    }

    bfs.push(start);

    // Second trace to visit needed nodes in topological order.
    while (!bfs.empty()) {
      GateID cur = bfs.front();

      if (accessed.find(cur) != accessed.end()) {
        if (checkVisited(accessed, cur, forward)) {

          accessed.erase(cur);
          auto next = getNext(cur, forward);

          switch (callVisitor(cur)) {
            case FINISH_ALL:
              return;
            case FINISH_THIS:
              bfs.pop();
              continue;
            case SUCCESS:
              break;
          }

          for (auto node: next) {
            bfs.push(node);
          }
        } else {
          auto prev = getNext(cur, !forward);
          for (auto node: prev) {
            if (accessed.find(node) != accessed.end()) {
              bfs.push(node);
            }
          }
          bfs.push(cur);
        }
      }
      bfs.pop();
    }
  }

  void Walker::walk(Walker::GateID start, bool forward) {
    std::queue<GateID> bfs;
    bfs.push(start);

    std::unordered_set<GateID> accessed;

    // First trace to define needed nodes.
    while (!bfs.empty()) {
      GateID cur = bfs.front();
      bfs.pop();
      accessed.emplace(cur);
      auto next = getNext(cur, forward);
      for (auto node: next) {
        bfs.push(node);
      }
    }

    bfs.push(start);

    // Second trace to visit needed nodes in topological order.
    while (!bfs.empty()) {
      GateID cur = bfs.front();

      if (accessed.find(cur) != accessed.end()) {
        if (checkVisited(accessed, cur, forward)) {

          accessed.erase(cur);
          auto next = getNext(cur, forward);

          switch (callVisitor(cur)) {
            case FINISH_ALL:
              return;
            case FINISH_THIS:
              bfs.pop();
              continue;
            case SUCCESS:
              break;
          }

          for (auto node: next) {
            bfs.push(node);
          }
        } else {
          auto prev = getNext(cur, !forward);
          for (auto node: prev) {
            if (accessed.find(node) != accessed.end()) {
              bfs.push(node);
            }
          }
          bfs.push(cur);
        }
      }
      bfs.pop();
    }
  }

  VisitorFlags Walker::callVisitor(GateID node) {
    auto flag = visitor->onNodeBegin(node);
    if (flag != VisitorFlags::SUCCESS) {
      return flag;
    }

    if (cutStorage) {
      auto &cuts = cutStorage->cuts[node];
      for (const auto &cut: cuts) {
        auto status = visitor->onCut(cut);
        if (status == FINISH_THIS) {
          break;
        } else if (status != SUCCESS) {
          return status;
        }
      }
    }

    return visitor->onNodeEnd(node);
  }

  bool Walker::checkVisited(std::unordered_set<GateID> &accessed,
                            GateID node, bool forward) {
    if (forward) {
      const auto &inputs = Gate::get(node)->inputs();
      for (const auto &in: inputs) {
        if (accessed.find(in.node()) != accessed.end()) {
          return false;
        }
      }
    } else {
      const auto &outputs = Gate::get(node)->links();
      for (const auto &out: outputs) {
        if (accessed.find(out.target) != accessed.end()) {
          return false;
        }
      }
    }
    return true;
  }

  std::vector<GNet::GateId> Walker::getNext(GateID node, bool forward) {
    std::vector<GNet::GateId> next;
    if (forward) {
      const auto &outputs = Gate::get(node)->links();
      next.reserve(outputs.size());
      for (const auto &out: outputs) {
        next.emplace_back(out.target);
      }
    } else {
      const auto &inputs = Gate::get(node)->inputs();
      next.reserve(inputs.size());
      for (const auto &in: inputs) {
        next.emplace_back(in.node());
      }
    }
    return next;
  }

} // namespace eda::gate::optimizer
