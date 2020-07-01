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

  dimension2->push_back(10.1);
  dimension2->push_back(10.2);
  dimension2->push_back(10.3);
  dimension2->push_back(10.4);
  dimension2->push_back(10.5);

  inner_param->push_back(1);
  inner_param->push_back(2);
  inner_param->push_back(3);
  inner_param->push_back(4);
  inner_param->push_back(5);

  tinc::ParameterSpace ps;

  ps.registerMappedParameter(dimension1);
  ps.registerCondition(dimension2);
  ps.registerParameter(inner_param);

  tinc::CppProcessor processor;

  processor.setOutputFileNames({"output.txt"});

  processor.processingFunction = [&]() {
    std::string text =
        processor.configuration["dim1"].flagValueStr + " -- " +
        std::to_string(processor.configuration["dim2"].flagValueInt) + " -- " +
        std::to_string(processor.configuration["inner_param"].flagValueDouble);

    std::cout << "Writing: "
              << processor.runningDirectory() +
                     processor.getOutputFileNames()[0]
              << std::endl;
    std::ofstream f(processor.runningDirectory() +
                        processor.getOutputFileNames()[0],
                    std::ofstream::app);
    f << text << std::endl;
    f.close();

    return true;
  };

  ps.rootPath = "data/";

  // Now sweep the parameter space
  ps.sweep(processor);

  return 0;
}
