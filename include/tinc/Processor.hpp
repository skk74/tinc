#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "al/scene/al_PolySynth.hpp"

#include <string>
#include <vector>

namespace tinc {

// TODO move info about filenames to separate class

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
   * @brief Set the names of input files
   * @param outputFiles list of output file names.
   */
  void setInputFileNames(std::vector<std::string> inputFiles);

  /**
   * @brief Set the current directory for process to run in.
   */
  void setRunningDirectory(std::string directory);

  std::string inputDirectory() { return mInputDirectory; }
  std::string outputDirectory() { return mOutputDirectory; }
  std::string runningDirectory() { return mRunningDirectory; }

  std::string id;

protected:
  std::string mRunningDirectory;
  std::string mOutputDirectory{"cached_output/"};
  std::string mInputDirectory;
  std::vector<std::string> mOutputFileNames;
  std::vector<std::string> mInputFileNames;
};

} // namespace tinc

#endif // PROCESSOR_HPP
