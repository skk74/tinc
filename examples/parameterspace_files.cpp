#include "tinc/CppProcessor.hpp"
#include "tinc/ParameterSpace.hpp"

#include "al/io/al_File.hpp"

#include <fstream>

int main() {
  auto dimension1 = std::make_shared<tinc::ParameterSpaceDimension>("dim1");
  auto dimension2 = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
  auto inner_param =
      std::make_shared<tinc::ParameterSpaceDimension>("inner_param");

  dimension1->push_back(0.1, "A");
  dimension1->push_back(0.2, "B");
  dimension1->push_back(0.3, "C");
  dimension1->push_back(0.4, "D");
  dimension1->push_back(0.5, "E");
  dimension1->type = tinc::ParameterSpaceDimension::MAPPED;

  dimension2->push_back(10.1);
  dimension2->push_back(10.2);
  dimension2->push_back(10.3);
  dimension2->push_back(10.4);
  dimension2->push_back(10.5);
  dimension2->type = tinc::ParameterSpaceDimension::INDEX;

  inner_param->push_back(1);
  inner_param->push_back(2);
  inner_param->push_back(3);
  inner_param->push_back(4);
  inner_param->push_back(5);
  inner_param->type = tinc::ParameterSpaceDimension::INTERNAL;

  tinc::ParameterSpace ps;

  ps.registerDimension(dimension1);
  ps.registerDimension(dimension2);
  ps.registerDimension(inner_param);

  if (!ps.writeToNetCDF()) {
    std::cerr << "Error writing NetCDF file" << std::endl;
  }

  // Now load the parameter space from disk
  tinc::ParameterSpace ps2;

  ps2.readFromNetCDF();

  for (auto dimensionName : ps2.dimensionNames()) {
    std::cout << " ---- Dimension: " << dimensionName << std::endl;
    auto dim = ps2.getDimension(dimensionName);

    std::cout << "  Values: " << std::endl;
    for (auto value : dim->values()) {
      std::cout << value << std::endl;
    }
    if (dim->ids().size() > 0) {
      std::cout << "  Ids: " << std::endl;
      for (auto id : dim->ids()) {
        std::cout << id << std::endl;
      }
    }
  }

  return 0;
}
