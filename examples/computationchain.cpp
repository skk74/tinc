#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"

using namespace al;
using namespace tinc;

//   1    2
//   |    |
//   3    |
//   \   /
//     |
//     4

struct MyApp : public App {

  ComputationChain mainChain{};
  ComputationChain joinChain{ComputationChain::PROCESS_ASYNC};
  ComputationChain chain1_3;
  CppProcessor process1;
  CppProcessor process2;
  CppProcessor process3;
  CppProcessor process4;

  Parameter value{"value", "", 0.0, "", 0.0, 1.0};
  ControlGUI gui;

  float sourceValue;
  float data1;
  float data2;

  void onInit() override {

    // Define processing functions
    process1.processingFunction = [&]() {
      data1 = sourceValue + 1.0;
      al_sleep(0.5);
      std::cout << "Done processing 1" << std::endl;
      return true;
    };
    process1.id = "1";

    process2.processingFunction = [&]() {
      data2 = -1.0;
      al_sleep(sourceValue);
      std::cout << "Done processing 2" << std::endl;
      return true;
    };
    process2.id = "2";

    process3.processingFunction = [&]() {
      data1 += 1.0;
      al_sleep(0.3);
      std::cout << "Done processing 3" << std::endl;
      return true;
    };
    process3.id = "3";

    process4.processingFunction = [&]() {
      data1 = data1 + data2;
      al_sleep(0.1);
      std::cout << "Done processing 4" << std::endl;
      return true;
    };
    process4.id = "4";

    chain1_3 << process1 << process3;
    joinChain << chain1_3 << process2;
    mainChain << joinChain << process4;

    value.registerChangeCallback([&](float value) {
      sourceValue = value;
      mainChain.process();
      std::cout << "value: " << value << " produces: " << data1 << std::endl;
    });

    gui << value;
    gui.init();
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    gui.draw(g);
  }

  void onExit() override { gui.cleanup(); }
};

int main() {
  MyApp().start();

  return 0;
}
