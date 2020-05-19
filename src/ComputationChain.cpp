#include "tinc/ComputationChain.hpp"

#include <iostream>

using namespace tinc;

void ComputationChain::addProcessor(Processor &chain) {
  std::unique_lock<std::mutex> lk(mChainLock);
  switch (mType) {
  case PROCESS_ASYNC:
    // FIXME check if process is already async, so there's no need to do this.
    // FIXME free this on desgtructor
    mAsyncProcessesInternal.emplace_back(new ProcessorAsync(&chain));
    mProcesses.push_back(mAsyncProcessesInternal.back());
    break;
  case PROCESS_SERIAL:
    mProcesses.push_back(&chain);
    break;
  }
}

bool ComputationChain::process(bool forceRecompute) {
  bool ret = true;
  switch (mType) {
  case PROCESS_ASYNC:
    for (auto chain : mProcesses) {
      chain->process(forceRecompute);
    }
    for (auto chain : mProcesses) {
      ret &= ((ProcessorAsync *)chain)->waitUntilDone();
    }
    break;
  case PROCESS_SERIAL:
    for (auto chain : mProcesses) {
      ret &= chain->process(forceRecompute);
    }
    break;
  }
  return ret;
}
