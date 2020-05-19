#ifndef COMPUTATIONCHAIN_HPP
#define COMPUTATIONCHAIN_HPP

#include "tinc/DataScript.hpp"
#include "tinc/ProcessorAsync.hpp"

#include <mutex>
#include <thread>

namespace tinc {

class ComputationChain : public Processor {
public:
  typedef enum { PROCESS_SERIAL, PROCESS_ASYNC } ChainType;
  ComputationChain(ChainType type = PROCESS_SERIAL, std::string id = "")
      : Processor(id), mType(type) {}
  ComputationChain(std::string id) : Processor(id), mType(PROCESS_SERIAL) {}

  void addProcessor(Processor &chain);

  bool process(bool forceRecompute = false);

  void enable(bool e) { mEnabled = e; }

  ComputationChain &operator<<(Processor &processor) {
    addProcessor(processor);
    return *this;
  }
  std::vector<Processor *> processors() { return mProcessors; }

private:
  bool mEnabled{true};
  std::vector<Processor *> mProcessors;
  std::vector<ProcessorAsync *> mAsyncProcessesInternal;
  std::mutex mChainLock;
  ChainType mType;
};

} // namespace tinc

#endif // COMPUTATIONCHAIN_HPP
