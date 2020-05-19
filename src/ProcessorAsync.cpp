#include "al/system/al_Time.hpp"

#include "tinc/ProcessorAsync.hpp"

#include <iostream>
#include <thread>

using namespace tinc;

ProcessorAsync::ProcessorAsync(std::string id) : Processor(id) {
  startThread();
}

ProcessorAsync::ProcessorAsync(Processor *processor)
    : Processor(processor->id), mProcessor(processor) {
  startThread();
}

ProcessorAsync::~ProcessorAsync() {
  mRunning = false;
  mThread->join();
}

bool ProcessorAsync::process(bool forceRecompute) {
  {
    std::unique_lock<std::mutex> lk(mLock);
    //    std::cout << "start " << mProcessor->id << std::endl;
    mRequestForce = forceRecompute;
    //    std::cout << "notifying thread start " << mProcessor->id << std::endl;
    mCondVariable.notify_one();
  }
  {
    std::unique_lock<std::mutex> lk2(mStartLock);
    mStartCondVariable.wait(lk2);
  }
  return true;
}

bool ProcessorAsync::waitUntilDone() {
  // As soon as we can acquire the lock, the thread is waiting
  mLock.lock();
  //  std::cout << "done " << mProcessor->id << std::endl;
  mLock.unlock();
  return mRetValue;
}

void ProcessorAsync::startThread() {

  mThread = std::make_unique<std::thread>([this]() {
    std::unique_lock<std::mutex> lk(mLock);
    while (mRunning) {
      mCondVariable.wait(lk);
      {
        std::unique_lock<std::mutex> lk(mStartLock);
        mStartCondVariable.notify_one();
      }
      //      std::cout << "starting thread process " << mProcessor->id <<
      //      std::endl;
      if (mProcessor) {
        mRetValue = mProcessor->process(mRequestForce);
      } else {
        mRetValue = this->process(mRequestForce);
      }
      callDoneCallbacks(mRetValue);
      //      std::cout << "completed thread process " << mProcessor->id <<
      //      std::endl;
    }
  });
}

Processor *ProcessorAsync::processor() const { return mProcessor; }

void ProcessorAsync::setProcessor(Processor *processor) {
  mProcessor = processor;
}
