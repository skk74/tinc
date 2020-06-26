#ifndef PARAMETERSPACEDIMENSION_HPP
#define PARAMETERSPACEDIMENSION_HPP

#ifdef AL_WINDOWS
#define NOMINMAX
#include <Windows.h>
#undef far
#endif

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "al/ui/al_Parameter.hpp"

namespace tinc {

/**
 * @brief The ParameterSpaceDimension class maps parameter values to string ids
 *
 * This allows mapping continuous parameters to string ids, for example
 * for mapping to directory structures
 *
 * ParameterSpaces can be linked together using addConnectedParameterSpace()
 * to have the final ids be deduced from the combinations of two parameters
 *
 */
class ParameterSpaceDimension {
  friend class ParameterSpace;

public:
  ParameterSpaceDimension(std::string name, std::string group = "")
      : mParameterValue(name, group) {}
  std::string getName();

  // Access to current
  float getCurrentValue();
  void setCurrentValue(float value);

  size_t getCurrentIndex();
  std::string getCurrentId();

  // Multidimensional parameter spaces will result in single values having
  // multiple ids. This can be resolved externally using this function
  // but perhaps we should have a higher level class that solves this issue for
  // the general case
  std::vector<std::string> getAllCurrentIds();
  std::vector<size_t> getAllCurrentIndeces();

  // Access to specific elements

  // When using reverse = true, the value returned describes the
  // end of an open range, i.e, the index returned is one more than
  // an index that would match the value.
  size_t getFirstIndexForValue(float value, bool reverse = false);

  float at(size_t x);

  std::string idAt(size_t x);
  size_t getIndexForValue(float value);
  std::vector<std::string> getAllIds(float value);
  std::vector<size_t> getAllIndeces(float value);

  // Access to complete sets
  std::vector<float> values();
  std::vector<std::string> ids();

  // the parameter instance holds the current value.
  // You can set values for parameter space through this function
  // Register notifications and create GUIs/ network synchronization
  // Through this instance.
  al::Parameter &parameter();

  // Move parameter space

  void stepIncrement();
  void stepDecrease();

  //
  size_t size();
  void reserve(size_t totalSize);

  void sort();
  void clear();

  // There is no check to see if value is already present. Could cause trouble
  // if value is there already.
  void push_back(float value, std::string id = "");

  void append(float *values, size_t count, std::string idprefix = "");
  void append(int *values, size_t count, std::string idprefix = "");
  void append(uint32_t *values, size_t count, std::string idprefix = "");

  // Set limits from internal data
  void conform();

  void addConnectedParameterSpace(ParameterSpaceDimension *paramSpace);

  // Protect parameter space (to avoid access during modification)
  // TODO all readers above need to use this lock
  void lock() { mLock.lock(); }
  void unlock() { mLock.unlock(); }

private:
  // Data
  std::vector<float> mValues;
  std::vector<std::string> mIds;

  std::mutex mLock;

  // Current state
  al::Parameter mParameterValue;

  std::vector<ParameterSpaceDimension *> mConnectedSpaces;
};

} // namespace tinc

#endif // PARAMETERSPACEDIMENSION_HPP
