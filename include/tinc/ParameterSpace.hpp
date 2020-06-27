#ifndef PARAMETERSPACE_HPP
#define PARAMETERSPACE_HPP

#include "tinc/ParameterSpaceDimension.hpp"
#include "tinc/Processor.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace tinc {

class ParameterSpace {
public:
  ParameterSpace();

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
   * @brief Returns the names of all dimensions
   */
  std::vector<std::string> dimensions();

  /**
   * @brief Returns the names of all dimensions that affect filesystem
   * directories
   * @return
   *
   * Only mappedParameters and conditions are considered. Regular parameters are
   * considered not to affect filesystem location
   */
  std::vector<std::string> dimensionsForFilesystem();

  void sweep(Processor &processor, std::vector<std::string> dimensions = {},
             bool recompute = false);

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

  std::function<std::string(std::map<std::string, size_t>)>
      generateRelativePath = [&](std::map<std::string, size_t> indeces) {
        std::string path;
        for (auto dimensionSample : indeces) {
          auto dimension = getDimension(dimensionSample.first);
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

protected:
  std::function<void(float, ParameterSpaceDimension *)> mChangeCallback =
      [](float, ParameterSpaceDimension *) {};

private:
};
} // namespace tinc

#endif // PARAMETERSPACE_HPP
