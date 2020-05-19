#include "tinc/ParameterSpaceNode.hpp"

using namespace al;

bool ParameterSpaceNode::consumeMessage(osc::Message &m,
                                        std::string rootOSCPath) {
  static const char nodePrefix[] = "/__ParameterSpace/";
  if (m.addressPattern().substr(0, sizeof(nodePrefix)) == nodePrefix) {
    return true;
  }
}
