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

  ps.rootPath = "subdir_data/";
  ps.createDataDirectories();

  if (!ps.writeToNetCDF()) {
    std::cerr << "Error writing NetCDF file" << std::endl;
  }

  // Make a change for one  of the data directories in a dimension
  tinc::ParameterSpace subdirps;

  auto dimension2b = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
  dimension2b->push_back(20.1);
  dimension2b->push_back(20.2);
  dimension2b->push_back(20.3);
  dimension2b->push_back(20.4);
  dimension2b->push_back(20.5);
  dimension2b->type = tinc::ParameterSpaceDimension::INDEX;

  subdirps.registerDimension(dimension2b);

  subdirps.rootPath = "subdir_data/B/";
  subdirps.createDataDirectories();

  if (!subdirps.writeToNetCDF()) {
    std::cerr << "Error writing NetCDF file" << std::endl;
  }

  // Now load to confirm the variation in the subdirectory is picked up.
  tinc::ParameterSpace ps2;
  ps2.rootPath = "subdir_data/";
  ps2.readFromNetCDF();

  ps2.getDimension("dim1")->setCurrentIndex(0);

  auto value1 = ps.getDimension("dim2")->at(0);
  auto value2 = ps2.getDimension("dim2")->at(0);

  std::cout << value1 << " -- " << value2 << std::endl;

  // This should trigger read of parameter space
  ps2.getDimension("dim1")->setCurrentIndex(1);
  value2 = ps2.getDimension("dim2")->at(0);
  std::cout << value1 << " -- " << value2 << std::endl;

  // Should revert to root parameter space
  ps2.getDimension("dim1")->setCurrentIndex(0);
  value2 = ps2.getDimension("dim2")->at(0);
  std::cout << value1 << " -- " << value2 << std::endl;

  return 0;
}
