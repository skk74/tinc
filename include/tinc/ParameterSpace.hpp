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
    dimension->parameter().registerChangeCallback(
        [&](float value) { mChangeCallback(value, dimension.get()); });
    parameters.push_back(dimension);
  }

  void
  registerMappedParameter(std::shared_ptr<ParameterSpaceDimension> dimension) {
    dimension->parameter().registerChangeCallback(
        [&](float value) { mChangeCallback(value, dimension.get()); });
    mappedParameters.push_back(dimension);
  }

  void registerCondition(std::shared_ptr<ParameterSpaceDimension> dimension) {
    dimension->parameter().registerChangeCallback(
        [&](float value) { mChangeCallback(value, dimension.get()); });
    conditionParameters.push_back(dimension);
  }

  // These should not be modifed by the user (perhaps make private?)
  std::vector<std::shared_ptr<ParameterSpaceDimension>> parameters;
  std::vector<std::shared_ptr<ParameterSpaceDimension>>
      mappedParameters; // Have ids, map to filesystem
  std::vector<std::shared_ptr<ParameterSpaceDimension>>
      conditionParameters; // map to filesystem

  static ParameterSpace loadFromNetCDF(std::string ncFile);

  void registerChangeCallback(
      std::function<void(float, ParameterSpaceDimension *)> changeCallback) {
    mChangeCallback = changeCallback;
  }

private:
  std::function<void(float, ParameterSpaceDimension *)> mChangeCallback;
};
} // namespace tinc

#endif // PARAMETERSPACE_HPP
