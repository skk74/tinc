#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "al/scene/al_PolySynth.hpp"

#include <string>
#include <vector>

namespace tinc {

// TODO move PushDirectory to allolib? Or its own file?
class PushDirectory {
public:
  PushDirectory(std::string directory, bool verbose = false);

  ~PushDirectory();

private:
  char previousDirectory[512];
  bool mVerbose;

  static std::mutex mDirectoryLock; // Protects all instances of PushDirectory
};

enum FlagType {
  FLAG_INT = 0,
  FLAG_DOUBLE, // The script to be run
  FLAG_STRING
};

struct Flag {

  Flag() {}
  Flag(std::string value) {
    type = FLAG_STRING;
    flagValueStr = value;
  }
  Flag(const char *value) {
    type = FLAG_STRING;
    flagValueStr = value;
  }

  Flag(int64_t value) {
    type = FLAG_INT;
    flagValueInt = value;
  }

  Flag(double value) {
    type = FLAG_DOUBLE;
    flagValueDouble = value;
  }

  //  ~Flag()
  //  {
  //      delete[] cstring;  // deallocate
  //  }

  //  Flag(const Flag& other) // copy constructor
  //      : Flag(other.cstring)
  //  {}

  //  Flag(Flag&& other) noexcept // move constructor
  //      : cstring(std::exchange(other.cstring, nullptr))
  //  {}

  //  Flag& operator=(const Flag& other) // copy assignment
  //  {
  //      return *this = Flag(other);
  //  }

  //  Flag& operator=(Flag&& other) noexcept // move assignment
  //  {
  //      std::swap(cstring, other.cstring);
  //      return *this;
  //  }

  std::string commandFlag; // A prefix to the flag (e.g. -o)

  FlagType type;
  std::string flagValueStr;
  int64_t flagValueInt;
  double flagValueDouble;
};

// You must call prepareFunction(), callDoneCallbacks() and test for 'enabled'
// within the process() function of all child classes. ( Should we wrap this to
// avoid user error here? )
class Processor {
public:
  Processor(std::string id_ = "") : id(id_) {
    if (id_.size() == 0) {
      id = al::demangle(typeid(*this).name()) + "@" +
           std::to_string((uint64_t)this);
    }
  }
  virtual ~Processor() {}
  virtual bool process(bool forceRecompute = false) = 0;

  /**
   * @brief Convenience function to set input, output and running directory
   */
  void setDataDirectory(std::string directory);

  /**
   * @brief Set the directory for output files
   */
  void setOutputDirectory(std::string outputDirectory);

  /**
   * @brief Set the directory for input files
   */
  void setInputDirectory(std::string inputDirectory);

  /**
   * @brief Set the names of output files
   * @param outputFiles list of output file names.
   */
  void setOutputFileNames(std::vector<std::string> outputFiles);

  /**
   * @brief Query the current output filenames
   */
  std::vector<std::string> getOutputFileNames();

  /**
   * @brief Set the names of input files
   * @param outputFiles list of output file names.
   */
  void setInputFileNames(std::vector<std::string> inputFiles);

  // TODO how should the files synchronize to processing. Should these functions
  // return only available files, or should they reflect the current settings?
  // Perhaps we need two different functions
  /**
   * @brief Query the current input filenames
   */
  std::vector<std::string> getInputFileNames();

  /**
   * @brief Set the current directory for process to run in.
   */
  void setRunningDirectory(std::string directory);

  std::string inputDirectory() { return mInputDirectory; }
  std::string outputDirectory() { return mOutputDirectory; }
  std::string runningDirectory() { return mRunningDirectory; }

  void registerDoneCallback(std::function<void(bool)> func) {
    mDoneCallbacks.push_back(func);
  }

  void verbose(bool verbose = true) { mVerbose = verbose; }

  std::string id;
  bool ignoreFail{false}; ///< If set to true, processor chains will continue
                          ///< even if this processor fails. Has no effect if
                          ///< running asychronously
  bool enabled{true};

  /**
   * @brief Add configuration key value pairs here
   */
  std::map<std::string, Flag> configuration;

  /**
   * @brief Set a function to be called before computing to prepare data
   */
  std::function<bool(void)> prepareFunction;

  template <class ParameterType>
  Processor &registerParameter(al::ParameterWrapper<ParameterType> &param) {
    mParameters.push_back(&param);
    param.registerChangeCallback([&](ParameterType value) {
      configuration[param.getName()] = value;
      process();
    });
    return *this;
  }

  template <class ParameterType>
  Processor &operator<<(al::ParameterWrapper<ParameterType> &newParam) {
    return registerParameter(newParam);
  }

protected:
  std::string mRunningDirectory;
  std::string mOutputDirectory{"cached_output/"};
  std::string mInputDirectory;
  std::vector<std::string> mOutputFileNames;
  std::vector<std::string> mInputFileNames;
  bool mVerbose;

  std::vector<al::ParameterMeta *> mParameters;

  void callDoneCallbacks(bool result) {
    for (auto cb : mDoneCallbacks) {
      cb(result);
    }
  }

private:
  std::vector<std::function<void(bool)>> mDoneCallbacks;
};

} // namespace tinc

#endif // PROCESSOR_HPP
