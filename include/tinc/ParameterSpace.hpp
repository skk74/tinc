#ifndef PARAMETERSPACE_HPP
#define PARAMETERSPACE_HPP

#include "tinc/ParameterSpaceDimension.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace tinc {

class ParameterSpace {
public:
  ParameterSpace();

  std::shared_ptr<ParameterSpaceDimension> getDimension(std::string name) {

    if (parameterNameMap.find(name) != parameterNameMap.end()) {
      name = parameterNameMap[name];
    }
    for (auto ps : parameters) {
      if (ps->parameter().getName() == name) {
        return ps;
      }
    }
    for (auto ps : mappedParameters) {
      if (ps->parameter().getName() == name) {
        return ps;
      }
    }
    for (auto ps : conditionParameters) {
      if (ps->parameter().getName() == name) {
        return ps;
      }
    }
    return nullptr;
  }

  void registerParameter(std::shared_ptr<ParameterSpaceDimension> dimension) {
    parameters.push_back(dimension);
    ParameterSpaceDimension *dimensionPtr = dimension.get();
    dimension->parameter().registerChangeCallback(
        [&](float value) { mChangeCallback(value, dimensionPtr); });
  }

  void
  registerMappedParameter(std::shared_ptr<ParameterSpaceDimension> dimension) {
    mappedParameters.push_back(dimension);
    ParameterSpaceDimension *dimensionPtr = dimension.get();
    dimension->parameter().registerChangeCallback(
        [&](float value) { mChangeCallback(value, dimensionPtr); });
  }

  void registerCondition(std::shared_ptr<ParameterSpaceDimension> dimension) {
    conditionParameters.push_back(dimension);
    ParameterSpaceDimension *dimensionPtr = dimension.get();
    dimension->parameter().registerChangeCallback(
        [&](float value) { mChangeCallback(value, dimensionPtr); });
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

  static ParameterSpace loadFromNetCDF(std::string ncFile);

  void registerChangeCallback(
      std::function<void(float, ParameterSpaceDimension *)> changeCallback) {
    mChangeCallback = changeCallback;
  }

private:
  std::function<void(float, ParameterSpaceDimension *)> mChangeCallback =
      [](float, ParameterSpaceDimension *) {};
};
} // namespace tinc

#endif // PARAMETERSPACE_HPP
