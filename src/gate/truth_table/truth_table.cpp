 #include "truth_table.h"

namespace GModel = eda::gate::model;

using eda::gate::model::Gate;
using eda::gate::simulator::Simulator;

///Returns vector of means, where vector[0] is mean for 1st out of Gnet and etc.
std::vector<uint64_t> buildTruthTab(
    const GModel::GNet *net) {
  static Simulator simulator;
  Gate::LinkList in, out;
  for (auto link: net->sourceLinks()) {
    in.push_back(Gate::Link(link.target));
  }
  for(auto link: net->targetLinks()) {
    out.push_back(Gate::Link(link.source));
  }
  auto compiled = simulator.compile(*net, in, out);
  uint64_t mean;
  std::vector<uint64_t> tMean;
  tMean.resize(out.size());
  std::fill(tMean.begin(), tMean.end(), 0);
  uint64_t length = 1 << net->nSourceLinks();
  for (uint64_t i = 0; i < length; ++i) {
    compiled.simulate(mean, i);
    for (uint64_t j = 0; j < tMean.size(); ++j) {
      tMean[j] += ((mean >> j) % 2) << i;
    }
  }
  return tMean;
}
