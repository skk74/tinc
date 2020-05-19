#ifndef PROCESSSERVER_HPP
#define PROCESSSERVER_HPP

#include "al/protocol/al_OSC.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/Processor.hpp"

namespace tinc {

class ProcessorServer : public al::osc::PacketHandler {
public:
  ProcessorServer();

  void exposeToNetwork(al::ParameterServer &pserver) {
    pserver.appendCommandHandler(*this);
  }

  void registerProcessor(Processor &processor) {
    mProcessors.push_back(&processor);
  }

  ProcessorServer &operator<<(Processor &p) {
    registerProcessor(p);
    return *this;
  }

  void onMessage(al::osc::Message &m) override;

protected:
  std::vector<Processor *> mProcessors;
};

} // namespace tinc

#endif // PROCESSSERVER_HPP
