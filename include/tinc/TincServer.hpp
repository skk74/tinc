#ifndef TINCSERVER_HPP
#define TINCSERVER_HPP

#include "al/protocol/al_OSC.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/ParameterSpace.hpp"
#include "tinc/Processor.hpp"

namespace tinc {

class TincServer : public al::osc::PacketHandler {
public:
  TincServer();

  void exposeToNetwork(al::ParameterServer &pserver) {
    pserver.appendCommandHandler(*this);
  }

  void registerProcessor(Processor &processor) {
    mProcessors.push_back(&processor);
  }

  void registerParameterSpace(ParameterSpace &ps) {
    mParameterSpaces.push_back(&ps);
  }

  TincServer &operator<<(Processor &p) {
    registerProcessor(p);
    return *this;
  }

  TincServer &operator<<(ParameterSpace &p) {
    registerParameterSpace(p);
    return *this;
  }

  void onMessage(al::osc::Message &m) override;

protected:
  std::vector<Processor *> mProcessors;
  std::vector<ParameterSpace *> mParameterSpaces;
};

} // namespace tinc

#endif // TINCSERVER_HPP
