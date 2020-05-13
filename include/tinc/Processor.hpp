#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <string>
#include <vector>

namespace tinc {

class Processor {
public:
  Processor(std::string id = "") {
    if (id.size() == 0) {
      id = std::to_string((uint64_t)this);
    }
  }
  virtual ~Processor() {}
  virtual bool process(bool forceRecompute = false) = 0;
  /**
   * @brief Set the directory for output files
   */
  void setOutputDirectory(std::string outputDirectory);

  /**
   * @brief Set the current directory for process to run in.
   */
  void setRunningDirectory(std::string directory);

  /**
   * @brief Set the names of output file
   * @param outputFiles list of output file names.
   */
  void setOutputFileNames(std::vector<std::string> outputFiles);

  std::string id;

protected:
  std::string mRunningDirectory;
  std::string mOutputDirectory{"cached_output/"};
  std::vector<std::string> mOutputFileNames;
};

} // namespace tinc

#endif // PROCESSOR_HPP
