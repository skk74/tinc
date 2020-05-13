#ifndef PROCESSORASYNC_H
#define PROCESSORASYNC_H

#include "tinc/Processor.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace tinc {

class ProcessorAsync : public Processor {
public:
  ProcessorAsync(std::string id);

  ProcessorAsync(Processor *processor = nullptr);

  ~ProcessorAsync();

  ProcessorAsync(const ProcessorAsync &other) // copy constructor
      : ProcessorAsync(other.mProcessor) {}
  ProcessorAsync(ProcessorAsync &&other) noexcept // move constructor
      : mThread(std::exchange(other.mThread, nullptr)) {}
  ProcessorAsync &operator=(const ProcessorAsync &other) // copy assignment
  {
    return *this = ProcessorAsync(other);
  }
  ProcessorAsync &operator=(ProcessorAsync &&other) noexcept // move assignment
  {
    std::swap(mThread, other.mThread);
    return *this;
  }

  bool process(bool forceRecompute = false) override;
  bool waitUntilDone();

protected:
  void startThread();

private:
  Processor *mProcessor{nullptr};
  bool mRunning{true};
  bool mRequestForce{false};
  bool mRetValue;
  std::unique_ptr<std::thread> mThread;
  std::mutex mLock;
  std::condition_variable mCondVariable;

  std::mutex mStartLock;
  std::condition_variable mStartCondVariable;
};

} // namespace tinc

#endif // PROCESSORASYNC_H
