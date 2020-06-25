#include "tinc/ParameterSpace.hpp"

int main() {
  tinc::ParameterSpace ps;
  ps.loadFromNetCDF(
      "/Users/lt01/code/casm_viewer/vdv_data/NaCoO2_diffusion/monte_with_com/"
      "cached_output/_parameter_space.nc");

  for (auto dimension : ps.parameters) {
    std::cout << " ***** Parameter: " << dimension->getName()
              << " size: " << dimension->size() << std::endl;
    for (auto value : dimension->values()) {
      std::cout << value << " ";
    }
    std::cout << std::endl;
  }
  for (auto dimension : ps.conditionParameters) {
    std::cout << " ***** Condition Parameter: " << dimension->getName()
              << " size: " << dimension->size() << std::endl;
    for (auto value : dimension->values()) {
      std::cout << value << " ";
    }
    std::cout << std::endl;
  }
  for (auto dimension : ps.mappedParameters) {
    std::cout << " ***** Mapped Parameter: " << dimension->getName()
              << " size: " << dimension->size() << std::endl;
    for (auto value : dimension->values()) {
      std::cout << value << " ";
    }
    std::cout << std::endl;
    // Mapped parameters have ids
    std::cout << " -- ids:" << std::endl;
    for (auto id : dimension->ids()) {
      std::cout << "'" << id << "' ";
    }
    std::cout << std::endl;
  }
  return 0;
}
