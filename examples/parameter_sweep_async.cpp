#include "tinc/CppProcessor.hpp"
#include "tinc/ParameterSpace.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/io/al_File.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include <fstream>

using namespace al;
using namespace tinc;

struct MyApp : public App {

  ParameterSpace ps;
  CppProcessor processor;

  ControlGUI gui;

  std::string displayText;

  void defineParameterSpace() {
    auto dimension1 = std::make_shared<tinc::ParameterSpaceDimension>("dim1");
    auto dimension2 = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
    auto inner_param =
        std::make_shared<tinc::ParameterSpaceDimension>("inner_param");

    // Create large parameter space
    for (int i = 0; i < 20; i++) {
      dimension1->push_back(i, "L_" + std::to_string(i));
    }
    dimension1->conform();
    dimension1->type = tinc::ParameterSpaceDimension::MAPPED;

    for (int i = 0; i < 22; i++) {
      dimension2->push_back(i / 220.0);
    }
    dimension2->conform();
    dimension2->type = tinc::ParameterSpaceDimension::INDEX;

    for (int i = 0; i < 23; i++) {
      inner_param->push_back(10 + i);
    }
    inner_param->conform();
    inner_param->type = tinc::ParameterSpaceDimension::INTERNAL;

    ps.registerDimension(dimension1);
    ps.registerDimension(dimension2);
    ps.registerDimension(inner_param);

    ps.generateRelativeRunPath = [&](std::map<std::string, size_t> indeces) {
      return "asyncdata/";
    };
    // Create necessary filesystem directories to be populated by data
    ps.createDataDirectories();

    // Register callback after every process call in a parameter sweep
    ps.onSweepProcess = [&](std::map<std::string, size_t> currentIndeces,
                            double progress) {
      std::cout << "Progress: " << progress * 100 << "%" << std::endl;
    };

    // Whenever the parameter space point changes, this function is called
    ps.registerChangeCallback(
        [&](float /*value*/, ParameterSpaceDimension * /*changedDimension*/) {
          std::string name =
              "out_" + ps.getDimension("dim1")->getCurrentId() + "_" +
              std::to_string(ps.getDimension("dim2")->getCurrentIndex()) + "_" +
              std::to_string(
                  ps.getDimension("inner_param")->getCurrentValue()) +
              ".txt";
          std::ifstream f(ps.currentRunPath() + name);
          if (f.is_open()) {
            f.seekg(0, std::ios::end);
            displayText.reserve(f.tellg());
            f.seekg(0, std::ios::beg);

            displayText.assign((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());
          } else {
            displayText = "Sample not created";
          }
          return true;
        });
  }

  void initializeComputation() {

    processor.prepareFunction = [&]() {
      std::string name =
          "out_" + processor.configuration["dim1"].flagValueStr + "_" +
          std::to_string(processor.configuration["dim2"].flagValueInt) + "_" +
          std::to_string(
              processor.configuration["inner_param"].flagValueDouble) +
          ".txt";
      processor.setOutputFileNames({name});
      return true;
    };

    // processing function takes longer than one second
    processor.processingFunction = [&]() {
      std::string text =
          processor.configuration["dim1"].flagValueStr + " -- " +
          std::to_string(processor.configuration["dim2"].flagValueInt) +
          " -- " +
          std::to_string(
              processor.configuration["inner_param"].flagValueDouble);

      std::ofstream f(processor.outputDirectory() +
                      processor.getOutputFileNames()[0]);
      f << text << std::endl;
      f.close();
      std::cout << "Wrote "
                << processor.outputDirectory() +
                       processor.getOutputFileNames()[0]
                << std::endl;
      al_sleep(0.5);
      return true;
    };
  }

  void onInit() override {
    defineParameterSpace();
    initializeComputation();

    gui << ps.getDimension("dim1")->parameter();
    gui << ps.getDimension("dim2")->parameter();
    gui << ps.getDimension("inner_param")->parameter();
    gui.init();

    // Now sweep the parameter space asynchronously
    ps.sweepAsync(processor);
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    gui.draw(g);
    g.color(0);
    FontRenderer::render(g, displayText.c_str(), {-1, 0, -4}, 0.1);
  }

  void onExit() override {
    ps.stopSweep();
    gui.cleanup();
  }
};

int main() {
  MyApp().start();

  return 0;
}
