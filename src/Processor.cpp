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

void Processor::setInputDirectory(std::string inputDirectory) {
  mInputDirectory = al::File::conformPathToOS(inputDirectory);
  std::replace(mInputDirectory.begin(), mInputDirectory.end(), '\\', '/');
  if (!al::File::isDirectory(mInputDirectory)) {
    std::cout
        << "Warning input directory for Processor doesn't exist. Creating."
        << std::endl;
    if (!al::Dir::make(mInputDirectory)) {
      std::cout << "Unable to create input directory:" << mOutputDirectory
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
    // FIXME this is not being used everywhere it should be....
    mOutputFileNames.push_back(name);
  }
}

void Processor::setInputFileNames(std::vector<std::string> inputFiles) {
  mInputFileNames.clear();
  for (auto fileName : inputFiles) {
    auto name = al::File::conformPathToOS(fileName);
    // FIXME this is not being used everywhere it should be....
    mInputFileNames.push_back(name);
  }
}
