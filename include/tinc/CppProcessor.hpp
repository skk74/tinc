#ifndef CPPPROCESSOR_HPP
#define CPPPROCESSOR_HPP

#include "tinc/Processor.hpp"

#include <functional>

namespace tinc {

class CppProcessor : public Processor {
public:
  CppProcessor(std::string id = "");

  bool process(bool forceRecompute = true) override;

  std::function<bool(void)> processingFunction;

private:
};

} // namespace tinc

#endif // CPPPROCESSOR_HPP
