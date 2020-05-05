#ifndef DISKBUFFER_HPP
#define DISKBUFFER_HPP

#include <string>

#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/BufferManager.hpp"

namespace tinc {

template <class DataType> class DiskBuffer : public BufferManager<DataType> {
public:
  DiskBuffer(std::string name, std::string fileName = "", std::string path = "",
             uint16_t size = 2);

  virtual bool updateData(std::string filename = "");

  // Careful, this is not thread safe. Needs to be called synchronously to any
  // process functions
  std::string getCurrentFileName() { return m_fileName; }

  void exposeToNetwork(al::ParameterServer &p);

protected:
  virtual bool parseFile(std::ifstream &file,
                         std::shared_ptr<DataType> newData) = 0;

  // Make this function private as users should not have a way to make the
  // buffer writable. Data writing should be done by writing to the file.
  using BufferManager<DataType>::getWritable;

  std::string m_fileName;
  std::string m_name;
  std::string m_path;
  std::shared_ptr<al::ParameterString> m_trigger;
};

template <class DataType>
DiskBuffer<DataType>::DiskBuffer(std::string name, std::string fileName,
                                 std::string path, uint16_t size)
    : BufferManager<DataType>(size) {
  m_name = name;
  // TODO there should be a check through a singleton to make sure names are
  // unique
  m_fileName = fileName;
  if (path.size() > 0) {
    m_path = al::File::conformDirectory(path);
  } else {
    m_path = "";
  }
}

template <class DataType>
bool DiskBuffer<DataType>::updateData(std::string filename) {
  if (filename.size() > 0) {
    m_fileName = filename;
  }
  std::ifstream file(m_path + m_fileName);
  if (file.good()) {
    auto buffer = getWritable();
    bool ret = parseFile(file, buffer);
    BufferManager<DataType>::doneWriting(buffer);
    return ret;
  } else {
    std::cerr << "Error code: " << strerror(errno);
    return false;
  }
}

template <class DataType>
void DiskBuffer<DataType>::exposeToNetwork(al::ParameterServer &p) {
  if (m_trigger) {
    std::cerr << "ERROR: already registered. Aborting." << std::endl;
    return;
  }
  std::string pathPrefix = "/__DiskBuffer/";
  //    if (m_fileName[0] != '/') {
  //      pathPrefix += "/";
  //    }
  m_trigger = std::make_shared<al::ParameterString>(pathPrefix + m_name);
  p.registerParameter(*m_trigger);
  m_trigger->registerChangeCallback(
      [this](std::string value) { this->updateData(value); });
  // There will be problems if this object is destroyed before the parameter
  // server Should this be a concern?
}

} // namespace tinc

#endif // DISKBUFFER_HPP
