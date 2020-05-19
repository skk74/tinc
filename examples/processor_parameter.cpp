#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"

using namespace al;
using namespace tinc;

// This example shows how parameters can be connected to
// trigger computation chains

struct MyApp : public App {

  CppProcessor process{"Process"};

  Parameter value{"value", "", 0.0, "", 0.0, 1.0};
  ControlGUI gui;

  void onInit() override {

    // Register the parameter with the processor
    process << value;
    // Define processing function. The new value is avaialble through
    // the configuration member and the old value is available from the
    // parameter as the new value has no been applied at this point
    process.processingFunction = [&]() {
      std::cout << "new value: "
                << process.configuration["value"].flagValueDouble
                << " previous value " << value.get() << std::endl;
      return true;
    };

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
