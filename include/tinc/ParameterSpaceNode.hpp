#ifndef PARAMETERSPACENODE_HPP
#define PARAMETERSPACENODE_HPP

#include "al/protocol/al_OSC.hpp"

#include <string>

class ParameterSpaceNode : public al::osc::MessageConsumer {
public:
  ParameterSpaceNode(std::string id = "_") : mId(id) {}

  bool consumeMessage(al::osc::Message &m, std::string rootOSCPath) override;

private:
  std::string mId;
};

#endif // PARAMETERSPACENODE_HPP
