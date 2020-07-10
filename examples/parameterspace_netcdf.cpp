#include "tinc/ParameterSpace.hpp"

int main() {
  tinc::ParameterSpace ps;
  ps.readFromNetCDF(
      "/Users/lt01/code/casm_viewer/vdv_data/NaCoO2_diffusion/monte_with_com/"
      "cached_output/_parameter_space.nc");

  for (auto dimension : ps.dimensions) {
    if (dimension->type == tinc::ParameterSpaceDimension::INTERNAL) {
      std::cout << " ***** Internal Parameter: " << dimension->getName()
                << " size: " << dimension->size() << std::endl;
      for (auto value : dimension->values()) {
        std::cout << value << " ";
      }
      std::cout << std::endl;
    } else if (dimension->type == tinc::ParameterSpaceDimension::INDEX) {
      std::cout << " ***** Index Parameter: " << dimension->getName()
                << " size: " << dimension->size() << std::endl;
      for (auto value : dimension->values()) {
        std::cout << value << " ";
      }
      std::cout << std::endl;
    } else if (dimension->type == tinc::ParameterSpaceDimension::MAPPED) {
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
  }
  return 0;
}
