#include "tinc/Processor.hpp"

#include "al/io/al_File.hpp"

#include <iostream>

using namespace tinc;

void Processor::setOutputDirectory(std::string outputDirectory) {
  mOutputDirectory = al::File::conformPathToOS(outputDirectory);
  std::replace(mOutputDirectory.begin(), mOutputDirectory.end(), '\\', '/');
  if (!al::File::isDirectory(mOutputDirectory)) {
    if (!al::Dir::make(mOutputDirectory)) {
      std::cout << "Unable to create cache directory:" << mOutputDirectory
                << std::endl;
    }
  }
}

void Processor::setRunningDirectory(std::string directory) {
  mRunningDirectory = al::File::conformPathToOS(directory);
  std::replace(mRunningDirectory.begin(), mRunningDirectory.end(), '\\', '/');
  if (!al::File::exists(mRunningDirectory)) {
    if (!al::Dir::make(mRunningDirectory)) {
      std::cout << "Error creating directory: " << mRunningDirectory
                << std::endl;
    }
  }
}

void Processor::setOutputFileNames(std::vector<std::string> outputFiles) {
  mOutputFileNames.clear();
  for (auto fileName : outputFiles) {
    auto name = al::File::conformPathToOS(fileName);
    std::replace(mOutputDirectory.begin(), mOutputDirectory.end(), '\\', '/');
    // FIXME this is not being used everywhere it should be....
    mOutputFileNames.push_back(name);
  }
}
