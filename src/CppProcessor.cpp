#include "tinc/CppProcessor.hpp"

using namespace tinc;

CppProcessor::CppProcessor(std::string id) : Processor(id) {}

bool CppProcessor::process(bool forceRecompute) {
  if (enabled) {
    bool ret = processingFunction();
    callDoneCallbacks(ret);
    return ret;
  } else {
    return true;
  }
}
