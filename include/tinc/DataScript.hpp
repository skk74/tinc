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

/**
 * @brief The DataScript class
 *
 *
 * @code
 *

 *
 * @endcode
 *
 *
 * @codeline
 * python myscript.py output_dir option 3.14159 0.5
 *
 * In the /home/sweet/home directory.
 *
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
  static std::string sanitizeName(std::string output_name);

  bool processAsync(bool noWait = false,
                    std::function<void(bool)> doneCallback = nullptr);

  bool processAsync(std::map<std::string, std::string> options,
                    bool noWait = false,
                    std::function<void(bool)> doneCallback = nullptr);

  bool runningAsync();

  bool waitForAsyncDone();

  void maxAsyncProcesses(int num) { mMaxAsyncProcesses = num; }

  void verbose(bool verbose = true) { mVerbose = verbose; }

protected:
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
      processSpace(allVecs, vecIndex + 1, indeces);
    }
  }

  void start(std::vector<std::vector<std::string>> parameterSpace,
             std::vector<std::pair<unsigned, unsigned>> parameterRanges) {}

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
