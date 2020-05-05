#ifndef JSONDISKBUFFER_HPP
#define JSONDISKBUFFER_HPP

#include "tinc/DiskBuffer.hpp"

#include "nlohmann/json.hpp"

namespace tinc {

class JsonDiskBuffer : public DiskBuffer<nlohmann::json> {
public:
  JsonDiskBuffer(std::string name, std::string fileName = "",
                 std::string path = "", uint16_t size = 2)
      : DiskBuffer<nlohmann::json>(name, fileName, path, size) {}

protected:
  virtual bool parseFile(std::ifstream &file,
                         std::shared_ptr<nlohmann::json> newData) {

    //    try {
    *newData = nlohmann::json::parse(file);

    return true;
    //    } catch () {
    //      std::cerr << "ERROR: parsing file. Increase cache on writing side."
    //                << std::endl;
    //      return false;
    //    }
  }
};

} // namespace tinc

#endif // JSONDISKBUFFER_HPP
