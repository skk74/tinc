#include "tinc/CppProcessor.hpp"

using namespace tinc;

CppProcessor::CppProcessor(std::string id) : Processor(id) {}

bool CppProcessor::process(bool forceRecompute) {
  PushDirectory(mRunningDirectory, mVerbose);
  if (!enabled) {
    return true;
  }
  if (prepareFunction && !prepareFunction()) {
    std::cerr << "ERROR preparing processor: " << id << std::endl;
    return false;
  }
  bool ret = processingFunction();
  callDoneCallbacks(ret);
  return ret;
}
