#include "tinc/CppProcessor.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/ScriptProcessor.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/io/al_File.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_Texture.hpp"
#include <fstream>

using namespace al;
using namespace tinc;

/*
 * This file shows using TINC to create a parameter space and sweep across it
 * calling a python script to generate data.
 *
 * The process to realize this is:
 * 1) Defining the parameter space (see defineParameterSpace()). Here the 4
 * parameters in the dataset are defined as dimensions in the parameter space.
 * They are defined as MAPPED dimensions as the directories are constructed to
 * specific zero-padding used in the dataset. The id stored with each parameter
 * dimension value is what will be used to construct the path for each sample in
 * the parameter space.
 *
 * 2) Configure the parameter space by defining the function that generates a
 * path from the current values of the parameter space
 * (ParameterSpace::generateRelativeRunPath)
 *
 * 3) Initialize the computation, by setting the command, script name, output
 * name and any other details that need to be configured for the script to run
 * correctly, see initializeComputation().
 *
 * The sweep is realized in onInit() by calling ParameterSpace::sweep(). This
 * function takes a Processor as argument. This processor will be run for every
 * sample within the path produced by the generateRelativeRunPath function that
 * we defined.
 */

struct MyApp : public App {
  ParameterSpace ps;
  ScriptProcessor processor;

  std::string clean_double_to_string(double value) {
    std::string val_as_string = std::to_string(value);
    auto trimmed = std::unique(val_as_string.begin(), val_as_string.end());
    std::string single_zero(val_as_string.begin(), trimmed);
    if (single_zero.back() == '0') {
      single_zero.pop_back();
    }
    if (single_zero.back() == '.') {
      single_zero.push_back('0');
    }
    return single_zero;
  }

  void defineParameterSpace() {
    auto eci1_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci1");
    auto eci2_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci2");
    auto eci3_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci3");
    auto eci4_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci4");

    // Create large parameter space
    std::vector<double> eci1_values = {-0.25, -0.125, 0.0, 0.125, 0.25};
    std::vector<double> eci2_values = {0.0, 0.5, 1.0, 1.5, 2.5};
    std::vector<double> eci3_values = {0.0, 0.5, 1.0, 1.5};
    std::vector<double> eci4_values = {0.0, 0.25, 0.5};
    for (const auto &val : eci1_values) {
      eci1_dim->push_back(val, "_" + clean_double_to_string(val));
    }
    eci1_dim->conform();
    eci1_dim->type = ParameterSpaceDimension::MAPPED;
    for (const auto &val : eci2_values) {
      eci2_dim->push_back(val, "_" + clean_double_to_string(val));
    }
    eci2_dim->type = ParameterSpaceDimension::MAPPED;
    eci2_dim->conform();
    for (const auto &val : eci3_values) {
      eci3_dim->push_back(val, "_" + clean_double_to_string(val));
    }
    eci3_dim->type = ParameterSpaceDimension::MAPPED;
    eci3_dim->conform();
    for (const auto &val : eci4_values) {
      eci4_dim->push_back(val, "_" + clean_double_to_string(val));
    }
    eci4_dim->type = ParameterSpaceDimension::MAPPED;
    eci4_dim->conform();

    ps.registerDimension(eci1_dim);
    ps.registerDimension(eci2_dim);
    ps.registerDimension(eci3_dim);
    ps.registerDimension(eci4_dim);

    // This function provided with a map of parameter name to index into
    // that parameter knows how to find the folder to run a process from
    ps.generateRelativeRunPath = [&](std::map<std::string, size_t> indeces) {
      std::string path = "AMX2_spinel_diffusion_0.0_0.0";
      for (const auto &mapped_param : ps.dimensions) {
        path +=
            mapped_param->idAt(indeces[mapped_param->parameter().getName()]);
      }
      return path + "/";
    };
    // Create necessary filesystem directories to be populated by data
    ps.createDataDirectories();

    // Register callback after every process call in a parameter sweep
    ps.onSweepProcess = [&](std::map<std::string, size_t> currentIndeces,
                            double progress) {
      std::cout << "Processed: " << ps.generateRelativeRunPath(currentIndeces)
                << std::endl;
      std::cout << "Progress: " << progress * 100 << "%" << std::endl;
    };
  }

  void initializeComputation() {
    // set local filename here relative to running directory and output
    // directory
    processor.setCommand("python");
    processor.setScriptName("../../plot_voltage.py");
    processor.setOutputFileNames({"voltage_curve.png"});
    // need to assign processor.registerDoneCallback to load image
    //(should be able to use running/output directory to find file)
  }

  void onInit() override {
    defineParameterSpace();
    initializeComputation();

    // Sweep the parameter space running 'processor' on every sample
    ps.sweep(processor);
  }

  void onDraw(Graphics &g) override { g.clear(0); }
};

int main() {
  MyApp().start();

  return 0;
}
