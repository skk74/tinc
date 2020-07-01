#ifndef PARAMETERSPACE_HPP
#define PARAMETERSPACE_HPP

#include "tinc/ParameterSpaceDimension.hpp"
#include "tinc/Processor.hpp"

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace tinc {

class ParameterSpace {
public:
  ParameterSpace();

  ~ParameterSpace();

  std::shared_ptr<ParameterSpaceDimension> getDimension(std::string name);

  void registerParameter(std::shared_ptr<ParameterSpaceDimension> dimension);

  void
  registerMappedParameter(std::shared_ptr<ParameterSpaceDimension> dimension);

  void registerCondition(std::shared_ptr<ParameterSpaceDimension> dimension);

  /**
   * @brief Returns all the paths that are used by the whole parameter space
   */
  std::vector<std::string> paths();

  /**
   * @brief Get relative filesystem path for current parameter values
   * @return
   *
   * Generated according to generateRelativePath()
   */
  std::string currentPath();

  /**
   * @brief Returns the names of all dimensions
   */
  std::vector<std::string> dimensions();

  /**
   * @brief Returns the names of all dimensions that affect filesystem
   * directories
   * @return
   *
   * Only mappedParameters and conditions are considered. Regular parameters
   * are considered not to affect filesystem location
   */
  std::vector<std::string> dimensionsForFilesystem();

  void sweep(Processor &processor, std::vector<std::string> dimensions = {},
             bool recompute = false);

  void sweepAsync(Processor &processor,
                  std::vector<std::string> dimensions = {},
                  bool recompute = false);

  /**
   * @brief Create necessary filesystem directories to be populated by data
   * @return true if successfully created (or checked existence) of
   * directories.
   */
  bool createDataDirectories();

  void stopSweep() {
    mSweepRunning = false;
    if (mAsyncProcessingThread) {
      mAsyncProcessingThread->join();
      mAsyncProcessingThread = nullptr;
    }
  }

  // These should not be modifed by the user (perhaps make private?)
  std::vector<std::shared_ptr<ParameterSpaceDimension>> parameters;
  std::vector<std::shared_ptr<ParameterSpaceDimension>>
      mappedParameters; // Have ids, map to filesystem
  std::vector<std::shared_ptr<ParameterSpaceDimension>>
      conditionParameters; // map to filesystem

  // To map names provided to getDimension() to internal data names
  // You can also use this map to display user friendly names when displaying
  // parameters
  std::map<std::string, std::string> parameterNameMap;

  // FIXME this interface is a little uncomfortable and not consistent with
  // the other similar functionality in setOuputFilename(), perhaps an output
  // filename generator function should be provided?
  std::function<std::string(std::map<std::string, size_t>)>
      generateRelativePath = [&](std::map<std::string, size_t> indeces) {
        std::string path;
        for (auto dimensionSample : indeces) {
          std::shared_ptr<ParameterSpaceDimension> dimension;
          for (auto ps : mappedParameters) {
            if (ps->parameter().getName() == dimensionSample.first) {
              dimension = ps;
              break;
            }
          }
          for (auto ps : conditionParameters) {
            if (ps->parameter().getName() == dimensionSample.first) {
              dimension = ps;
              break;
            }
          }
          if (dimension) {
            auto id = dimension->idAt(dimensionSample.second);
            path += id + "/";
          }
        }
        return path;
      };

  void loadFromNetCDF(std::string ncFile);

  void registerChangeCallback(
      std::function<void(float, ParameterSpaceDimension *)> changeCallback) {
    mChangeCallback = changeCallback;
  }

  std::string rootPath;

  std::function<void(std::map<std::string, size_t> currentIndeces,
                     double progress)>
      onSweepProcess;

protected:
  std::function<void(float newValue, ParameterSpaceDimension *changedDimension)>
      mChangeCallback = [](float, ParameterSpaceDimension *) {};

  std::unique_ptr<std::thread> mAsyncProcessingThread;

  bool mSweepRunning{false};

private:
};
} // namespace tinc

#endif // PARAMETERSPACE_HPP
