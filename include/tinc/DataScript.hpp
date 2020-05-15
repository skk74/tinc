#ifndef DATASCRIPT_HPP
#define DATASCRIPT_HPP

#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/Processor.hpp"

#include "nlohmann/json.hpp"

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace tinc {

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

class PushDirectory {
public:
  PushDirectory(std::string directory, bool verbose = false);

  ~PushDirectory();

private:
  char previousDirectory[512];
  bool mVerbose;

  static std::mutex mDirectoryLock; // Protects all instances of PushDirectory
};

/**
 * @brief The DataScript class
 *
 * You can use the DataScript class in two ways. First, you can use the
 * appendFlag() to add sequential flags to pass to the
 * script on the command line. The simplest usage looks like:
 *
 * DataScript ds("output_folder");
 * ds.setCommand("python");
 * ds.setRunningDirectory("/home/sweet/home");
 * ds.appendFlag("myscript.py", FLAG_SCRIPT);
 * ds.appendFlag("option");
 * ds.appendFlag("3.14159");
 * ds.process();
 *
 * You can override the configure() function for better control of the
 * passed arguments. This is a quick and easy way to set things up
 * and will work well when there is a small known set of parameters. You can
 * define any additional functions to get and set configuration options.
 *
 * @code
 *
 * class MyScript : public DataScript {
 * public:
 *     float value;
 *     void configure() override {
 *         setCommand("python");
 *         setRunningDirectory("/home/sweet/home");
 *         clearFlags();
 *         appendFlag("myscript.py", FLAG_SCRIPT);
 *         appendFlag("output_dir", FLAG_OUTPUT_DIR);
 *         appendFlag("option");
 *         appendFlag("3.14159");
 *         appendFlag(std::to_string(value));
 *     }
 * };
 *
 * void process() {
 *     DataScript ds;
 *     ds.value = 0.5;
 *     ds.process();
 *     std::string
 * }
 *
 * @endcode
 *
 * This will execute the command:
 *
 * @codeline
 * python myscript.py output_dir option 3.14159 0.5
 *
 * In the /home/sweet/home directory.
 *
 * For more complex scenarios, use the setConfiguration() function. This
 * will create a json file with the options that can then be read by the script
 * This provides the greatest flexibility and extensibility.
 */
class DataScript : public Processor {
public:
  // TODO change constructor to match Processor constructor
  DataScript(std::string id = "") : Processor(id) {}

  virtual ~DataScript() { waitForAsyncDone(); }

  /**
   * @brief Set the script's main command (e.g. python)
   */
  void setCommand(std::string command) { mScriptCommand = command; }

  /**
   * @brief Set name of script to be run
   * @param scriptName
   *
   * This name will be used unless a flag is set with type FLAG_SCRIPT
   */
  void setScriptName(std::string scriptName) {
    std::replace(mScriptName.begin(), mScriptName.end(), '\\', '/');
    mScriptName = scriptName;
  }

  /**
   * @brief Query current script file name
   */
  std::string scriptFile(bool fullPath = false);

  /**
   * @brief Query input file name
   */
  std::string inputFile(bool fullPath = true, int index = 0);

  /**
   * @brief Query output file name
   */
  std::string outputFile(bool fullPath = true, int index = 0);

  /**
   * @brief process
   */
  bool process(bool forceRecompute = false) override;

  /**
   * @brief Cleans a name up so it can be written to disk
   * @param output_name the source name
   * @return the cleaned up name
   *
   * This function will remove any characters not allowed by the operating
   * system like :,/,\ etc. And will remove any characters like '.' that
   * can confuse the parsing of the name on read.
   */
  std::string sanitizeName(std::string output_name);

  bool processAsync(bool noWait = false,
                    std::function<void(bool)> doneCallback = nullptr);

  bool processAsync(std::map<std::string, std::string> options,
                    bool noWait = false,
                    std::function<void(bool)> doneCallback = nullptr);

  bool runningAsync();

  bool waitForAsyncDone();

  void maxAsyncProcesses(int num) { mMaxAsyncProcesses = num; }

  void verbose(bool verbose = true) { mVerbose = true; }

  void registerDoneCallback() { throw "Not implemented yet."; }

  DataScript &registerParameter(al::ParameterMeta &param) {
    mParameters.push_back(&param);
    return *this;
  }

  DataScript &operator<<(al::ParameterMeta &newParam) {
    return registerParameter(newParam);
  }

  /**
   * @brief Add configuration key value pairs here
   */
  std::map<std::string, Flag> configuration;

protected:
  // These need to be accessible by the subclass
  std::vector<al::ParameterMeta *> mParameters;
  al::ParameterServer *mParamServer;

  std::string writeJsonConfig();

  void parametersToConfig(nlohmann::json &j);

private:
  std::string mScriptCommand{"/usr/bin/python3"};
  std::string mScriptName;

  std::mutex mProcessingLock;
  bool mVerbose;
  int mMaxAsyncProcesses{4};
  std::atomic<int> mNumAsyncProcesses{0};
  std::vector<std::thread> mAsyncThreads;
  std::thread mAsyncDoneThread;
  std::condition_variable mAsyncDoneTrigger;
  std::mutex mAsyncDoneTriggerLock;

  std::function<void(bool ok)> mDoneCallback;

  std::string makeCommandLine();

  bool runCommand(const std::string &command);

  bool writeMeta();

  al_sec modified(const char *path) const;

  bool needsRecompute();

  std::string metaFilename();
};

class CacheManager {
public:
  void registerProcessor(Processor &processor) {
    mProcessors.push_back(&processor);
  }

  void setRunningDirectory(std::string outputDirectory) {
    for (auto *processor : mProcessors) {
      processor->setRunningDirectory(outputDirectory);
    }
  }

  void setOutputDirectory(std::string outputDirectory) {
    for (auto *processor : mProcessors) {
      processor->setOutputDirectory(outputDirectory);
    }
  }

  void clearCache() {
    throw "Mot implemented yet.";
    // TODO implement clear cache
  }

private:
  std::vector<Processor *> mProcessors;
};

class ParallelProcessor : public DataScript {
public:
  // TODO is this worth finishing?

  void processSpace(const std::vector<std::vector<std::string>> &allVecs,
                    size_t vecIndex,
                    std::vector<size_t> indeces = std::vector<size_t>()) {
    if (vecIndex >= allVecs.size()) {
      return;
    }
    if (indeces.size() == 0) {
      indeces.resize(allVecs.size(), 0);
    }
    std::vector<std::string> currentValues(allVecs.size());
    auto indecesIt = indeces.begin();
    auto currentValuesIt = currentValues.begin();
    for (auto allVecsIt = allVecs.begin(); allVecsIt != allVecs.end();
         allVecsIt++) {
      *currentValuesIt = (*allVecsIt)[*indecesIt];
      indecesIt++;
      currentValuesIt++;
    }
    for (size_t i = 0; i < allVecs[vecIndex].size(); i++) {
      indeces[vecIndex] = i;
      auto value = allVecs[vecIndex][i];
      //            labelProcessor.setParams(chempot, std::to_string(i),
      //            datasetId, mConfig.pythonScriptPath);
      //            labelProcessor.process();
      processSpace(allVecs, vecIndex + 1, indeces);
    }
  }

  void start(std::vector<std::vector<std::string>> parameterSpace,
             std::vector<std::pair<unsigned, unsigned>> parameterRanges) {
    //        mParallelProcess = std::make_shared<std::thread>(
    //                    [this](parameterSpace,parameterRanges) {
    //                for (auto parameterValues: parameterSpace) {
    //                auto interator =
    //    }

    //    });
  }

  void waitForEnd() {}

  void stop() {}

private:
  std::shared_ptr<std::thread> mParallelProcess;
  std::atomic<bool> mRunning;
};

} // namespace tinc

#ifdef AL_WINDOWS
#undef popen
#undef pclose
#endif

#endif // DATASCRIPT_HPP
