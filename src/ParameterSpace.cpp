#include "tinc/ParameterSpace.hpp"

#include "al/io/al_File.hpp"

#ifdef TINC_HAS_HDF5
#include <netcdf.h>
#endif

#include <iostream>

using namespace tinc;

ParameterSpace::ParameterSpace() {}

ParameterSpace::~ParameterSpace() { stopSweep(); }

std::shared_ptr<ParameterSpaceDimension>
ParameterSpace::getDimension(std::string name) {

  if (parameterNameMap.find(name) != parameterNameMap.end()) {
    name = parameterNameMap[name];
  }
  for (auto ps : dimensions) {
    if (ps->parameter().getName() == name) {
      return ps;
    }
  }
  return nullptr;
}

void ParameterSpace::registerDimension(
    std::shared_ptr<ParameterSpaceDimension> dimension) {
  for (size_t i = 0; i < dimensions.size(); i++) {
    if (dimensions[i]->getName() == dimension->getName()) {
      dimensions[i]->mValues = dimension->values();
      dimensions[i]->mIds = dimension->ids();
      dimensions[i]->mConnectedSpaces = dimension->mConnectedSpaces;
      dimensions[i]->datatype = dimension->datatype;

      //      std::cout << "Clobbered dimension: " << dimension->getName() <<
      //      std::endl;
      // TODO should we check if the dimension has already been registered?
      dimension->parameter().registerChangeCallback(
          [dimension, this](float value) {
            //    std::cout << value << dimension->getName() << std::endl;
            float oldValue = dimension->parameter().get();
            dimension->parameter().setNoCalls(value);

            this->updateParameterSpace(oldValue, dimension.get());
            this->mChangeCallback(oldValue, dimension.get());
            dimension->parameter().setNoCalls(oldValue);
          });
      return;
    }
  }
  dimension->parameter().registerChangeCallback([dimension, this](float value) {
    //    std::cout << value << dimension->getName() << std::endl;
    this->updateParameterSpace(value, dimension.get());
    this->mChangeCallback(value, dimension.get());
  });
  dimensions.push_back(dimension);
}

std::vector<std::string> ParameterSpace::runningPaths() {
  std::vector<std::string> paths;
  auto dimensionNames = dimensionsForFilesystem();

  std::map<std::string, size_t> currentIndeces;
  for (auto dimension : dimensionNames) {
    currentIndeces[dimension] = 0;
  }
  bool done = false;
  while (!done) {
    done = true;
    auto path = al::File::conformPathToOS(rootPath) +
                generateRelativeRunPath(currentIndeces);
    if (path.size() > 0) {
      paths.push_back(path);
    }
    done = incrementIndeces(currentIndeces);
  }
  return paths;
}

std::string ParameterSpace::currentRunPath() {
  std::map<std::string, size_t> indeces;
  for (auto ps : dimensions) {
    if (ps->type == ParameterSpaceDimension::MAPPED ||
        ps->type == ParameterSpaceDimension::INDEX) {
      indeces[ps->getName()] = ps->getCurrentIndex();
    }
  }
  return al::File::conformPathToOS(generateRelativeRunPath(indeces));
}

std::vector<std::string> ParameterSpace::dimensionNames() {
  std::vector<std::string> dimensionNames;
  for (auto ps : dimensions) {
    dimensionNames.push_back(ps->parameter().getName());
  }
  return dimensionNames;
}

std::vector<std::string> ParameterSpace::dimensionsForFilesystem() {
  std::vector<std::string> dimensionNames;
  for (auto ps : dimensions) {
    if (ps->type == ParameterSpaceDimension::MAPPED ||
        ps->type == ParameterSpaceDimension::INDEX) {
      dimensionNames.push_back(ps->parameter().getName());
    }
  }
  return dimensionNames;
}

void ParameterSpace::clear() {
  dimensions.clear();
  mSpecialDirs.clear();
}

bool ParameterSpace::incrementIndeces(
    std::map<std::string, size_t> &currentIndeces) {

  for (auto &dimensionIndex : currentIndeces) {
    auto dimension = getDimension(dimensionIndex.first);
    dimensionIndex.second++;
    if (dimensionIndex.second >= dimension->size()) {
      dimensionIndex.second = 0;
    } else {
      return false;
    }
  }
  return true;
}

void ParameterSpace::sweep(Processor &processor,
                           std::vector<std::string> dimensionNames_,
                           bool recompute) {
  uint64_t sweepCount = 0;
  uint64_t sweepTotal = 1;
  mSweepRunning = true;
  if (dimensionNames_.size() == 0) {
    dimensionNames_ = dimensionNames();
  }
  std::map<std::string, size_t> currentIndeces;
  for (auto dimensionName : dimensionNames_) {
    if (getDimension(dimensionName)) {
      currentIndeces[dimensionName] = 0;
      sweepTotal *= getDimension(dimensionName)->size();
    } else {
      std::cerr << __FUNCTION__
                << " ERROR: dimension not found: " << dimensionName
                << std::endl;
    }
  }
  bool done = false;
  while (!done && mSweepRunning) {
    for (auto ps : dimensions) {
      if (ps->type == ParameterSpaceDimension::INTERNAL) {
        processor.configuration[ps->getName()] =
            ps->at(currentIndeces[ps->getName()]);
      } else if (ps->type == ParameterSpaceDimension::MAPPED) {
        processor.configuration[ps->getName()] =
            ps->idAt(currentIndeces[ps->getName()]);
      } else if (ps->type == ParameterSpaceDimension::INDEX) {
        assert(currentIndeces[ps->getName()] <
               std::numeric_limits<int64_t>::max());
        processor.configuration[ps->getName()] =
            (int64_t)currentIndeces[ps->getName()];
      }
    }
    auto path = al::File::conformPathToOS(rootPath) +
                generateRelativeRunPath(currentIndeces);
    if (path.size() > 0) {
      // TODO allow fine grained options of what directory to set
      processor.setRunningDirectory(path);
    }
    if (!processor.process(recompute) && !processor.ignoreFail) {
      return;
    }
    sweepCount++;
    if (onSweepProcess) {
      onSweepProcess(currentIndeces, sweepCount / (double)sweepTotal);
    }
    done = incrementIndeces(currentIndeces);
  }
  mSweepRunning = false;
}

void ParameterSpace::sweepAsync(Processor &processor,
                                std::vector<std::string> dimensions,
                                bool recompute) {

  mAsyncProcessingThread = std::make_unique<std::thread>([=, &processor]() {
    //
    this->sweep(processor, dimensions, recompute);
  });
}

bool ParameterSpace::createDataDirectories() {
  for (auto path : runningPaths()) {
    if (!al::File::isDirectory(path)) {
      if (!al::Dir::make(path)) {
        return false;
      }
    }
  }
  return true;
}

void ParameterSpace::stopSweep() {
  mSweepRunning = false;
  if (mAsyncProcessingThread) {
    mAsyncProcessingThread->join();
    mAsyncProcessingThread = nullptr;
  }
}

bool readNetCDFValues(int grpid,
                      std::shared_ptr<ParameterSpaceDimension> pdim) {
  int retval;
  int varid;
  nc_type xtypep;
  int ndimsp;
  int dimidsp[32];
  size_t lenp;
  int *nattsp = nullptr;
  if ((retval = nc_inq_varid(grpid, "values", &varid))) {
    return false;
  }
  if ((retval = nc_inq_var(grpid, varid, nullptr, &xtypep, &ndimsp, dimidsp,
                           nattsp))) {
    return false;
  }
  if ((retval = nc_inq_dimlen(grpid, dimidsp[0], &lenp))) {
    return false;
  }
  // TODO cover all supported cases and report errors.
  if (xtypep == NC_FLOAT) {
    std::vector<float> data;
    data.resize(lenp);
    if ((retval = nc_get_var(grpid, varid, data.data()))) {
      return false;
    }
    pdim->append(data.data(), data.size());
  } else if (xtypep == NC_INT) {

    std::vector<int32_t> data;
    data.resize(lenp);
    if ((retval = nc_get_var(grpid, varid, data.data()))) {
      return false;
    }
    pdim->append(data.data(), data.size());
  } else if (xtypep == NC_UBYTE) {

    std::vector<uint8_t> data;
    data.resize(lenp);
    if ((retval = nc_get_var(grpid, varid, data.data()))) {
      return false;
    }
    pdim->append(data.data(), data.size());
  } else if (xtypep == NC_UINT) {

    std::vector<uint32_t> data;
    data.resize(lenp);
    if ((retval = nc_get_var(grpid, varid, data.data()))) {
      return false;
    }
    pdim->append(data.data(), data.size());
  }
  return true;
}

bool ParameterSpace::readDimensionsInNetCDFFile(
    std::string filename,
    std::vector<std::shared_ptr<ParameterSpaceDimension>> &newDimensions) {
  int ncid, retval;
  int num_state_grps;
  int state_grp_ids[16];
  int num_parameters;
  int parameters_ids[16];
  int num_conditions;
  int conditions_ids[16];
  nc_type xtypep;
  int ndimsp;
  int dimidsp[32];
  // FIXME need to check size of dimensions first
  int *nattsp = nullptr;
  size_t lenp;
  int varid;

  int internal_state_grpid;
  int parameters_grpid;
  int conditions_grpid;

  if ((retval = nc_open(filename.c_str(), NC_NOWRITE | NC_SHARE, &ncid))) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return false;
  }

  // Get main group ids
  if (nc_inq_grp_ncid(ncid, "internal_dimensions", &internal_state_grpid) ==
      0) {
    if (nc_inq_grps(internal_state_grpid, &num_state_grps, state_grp_ids)) {
      return false;
    }

    // Read internal states variable data
    for (int i = 0; i < num_state_grps; i++) {
      char groupName[32];
      // FIXME need to check size of name first
      if (nc_inq_grpname(state_grp_ids[i], groupName)) {
        return false;
      }
      std::shared_ptr<ParameterSpaceDimension> pdim =
          std::make_shared<ParameterSpaceDimension>(groupName);

      if (!readNetCDFValues(state_grp_ids[i], pdim)) {
        return false;
      }
      pdim->conform();
      pdim->type = ParameterSpaceDimension::INTERNAL;
      newDimensions.push_back(pdim);
      //    std::cout << "internal state " << i << ":" << groupName
      //              << " length: " << lenp << std::endl;
    }
  } else {
    std::cout << "No group 'internal_dimensions' in " << filename << std::endl;
  }

  if (nc_inq_grp_ncid(ncid, "mapped_dimensions", &parameters_grpid) == 0) {
    if (nc_inq_grps(parameters_grpid, &num_parameters, parameters_ids)) {
      return false;
    }

    // Process mapped parameters
    for (int i = 0; i < num_parameters; i++) {
      char parameterName[32];
      // FIXME need to check size of name first
      if (nc_inq_grpname(parameters_ids[i], parameterName)) {
        return false;
      }
      if ((retval = nc_inq_varid(parameters_ids[i], "values", &varid))) {
        return false;
      }

      if ((retval = nc_inq_var(parameters_ids[i], varid, nullptr, &xtypep,
                               &ndimsp, dimidsp, nattsp))) {
        return false;
      }
      if ((retval = nc_inq_dimlen(parameters_ids[i], dimidsp[0], &lenp))) {
        return false;
      }
      std::vector<float> data;
      data.resize(lenp);
      if ((retval = nc_get_var(parameters_ids[i], varid, data.data()))) {
        return false;
      }
      // Now get ids
      if ((retval = nc_inq_varid(parameters_ids[i], "ids", &varid))) {
        return false;
      }

      if ((retval = nc_inq_var(parameters_ids[i], varid, nullptr, &xtypep,
                               &ndimsp, dimidsp, nattsp))) {
        return false;
      }
      if ((retval = nc_inq_dimlen(parameters_ids[i], dimidsp[0], &lenp))) {
        return false;
      }
      char totalData[lenp * 80];
      std::vector<char *> idData;

      idData.resize(lenp);
      for (int i = 0; i < lenp; i++) {
        idData[i] = totalData + (i * 80);
      }
      if ((retval =
               nc_get_var_string(parameters_ids[i], varid, idData.data()))) {
        return false;
      }

      std::shared_ptr<ParameterSpaceDimension> pdim =
          std::make_shared<ParameterSpaceDimension>(parameterName);
      pdim->append(data.data(), data.size());
      pdim->mIds.resize(lenp);
      for (int i = 0; i < lenp; i++) {
        pdim->mIds[i] = idData[i];
      }

      pdim->conform();
      pdim->type = ParameterSpaceDimension::MAPPED;
      newDimensions.push_back(pdim);

      //      std::cout << "mapped parameter " << i << ":" << parameterName
      //                << " length: " << lenp << std::endl;
    }
  } else {
    std::cerr << "Error finding group 'mapped_dimensions' in " << filename
              << std::endl;
  }

  if (nc_inq_grp_ncid(ncid, "index_dimensions", &conditions_grpid) == 0) {
    if (nc_inq_grps(conditions_grpid, &num_conditions, conditions_ids)) {
      return false;
    }
    // Read conditions
    for (int i = 0; i < num_conditions; i++) {
      char conditionName[32];
      // FIXME need to check size of name first
      if (nc_inq_grpname(conditions_ids[i], conditionName)) {
        return false;
      }
      std::shared_ptr<ParameterSpaceDimension> pdim =
          std::make_shared<ParameterSpaceDimension>(conditionName);

      if (!readNetCDFValues(conditions_ids[i], pdim)) {
        return false;
      }

      pdim->conform();
      pdim->type = ParameterSpaceDimension::INDEX;
      newDimensions.push_back(pdim);
    }
  } else {
    std::cerr << "Error finding group 'index_dimensions' in " << filename
              << std::endl;
  }

  //  mParameterSpaces["time"]->append(timeSteps.data(), timeSteps.size());
  if ((retval = nc_close(ncid))) {
    return false;
  }
  return true;
}

bool ParameterSpace::readFromNetCDF(std::string ncFile) {
#ifdef TINC_HAS_NETCDF
  std::vector<std::shared_ptr<ParameterSpaceDimension>> newDimensions;
  std::string filename = al::File::conformPathToOS(rootPath) + ncFile;
  if (!readDimensionsInNetCDFFile(filename, newDimensions)) {
    return false;
  }

  for (auto newDim : newDimensions) {
    registerDimension(newDim);
  }

  auto dimNames = dimensionNames();

  std::map<std::string, size_t> currentIndeces;
  for (auto dimension : dimNames) {
    currentIndeces[dimension] = 0;
  }
  bool done = false;
  while (!done) {
    auto path = generateRelativeRunPath(currentIndeces);

    std::stringstream ss(path);
    std::string item;
    std::string subPath;
    while (std::getline(ss, item, AL_FILE_DELIMITER)) {
      subPath += item + AL_FILE_DELIMITER_STR;
      if (al::File::exists(al::File::conformPathToOS(rootPath) + subPath +
                           ncFile)) {
        mSpecialDirs[subPath] = ncFile;
      }
    }

    done = incrementIndeces(currentIndeces);
  }

#else
  std::cerr << "TINC built without NetCDF support. "
               "ParameterSpaceDimension::loadFromNetCDF() does not work."
            << std::endl;
#endif
  return true;
}

bool writeNetCDFValues(int datagrpid,
                       std::shared_ptr<ParameterSpaceDimension> ps) {
  int retval;
  int shuffle = 1;
  int deflate = 9;
  int dimid;
  int varid;
  if ((retval = nc_def_dim(datagrpid, "values_dim", ps->size(), &dimid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  if (ps->datatype == ParameterSpaceDimension::FLOAT) {
    int dimidsp[1] = {dimid};
    if ((retval =
             nc_def_var(datagrpid, "values", NC_FLOAT, 1, dimidsp, &varid))) {

      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }
    if ((retval = nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }

    std::vector<float> values = ps->values();
    nc_put_var(datagrpid, varid, values.data());
  } else if (ps->datatype == ParameterSpaceDimension::UINT8) {

    int dimidsp[1] = {dimid};
    if ((retval =
             nc_def_var(datagrpid, "values", NC_UBYTE, 1, dimidsp, &varid))) {

      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }
    if ((retval = nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }

    // FIXME we should have an internal representation of the data space in the
    // original type instead of transforming
    std::vector<float> values = ps->values();
    std::vector<uint8_t> valuesInt;
    valuesInt.resize(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      valuesInt[i] = values.size();
    }

    nc_put_var(datagrpid, varid, valuesInt.data());

  } else if (ps->datatype == ParameterSpaceDimension::INT32) {

    int dimidsp[1] = {dimid};
    if ((retval =
             nc_def_var(datagrpid, "values", NC_INT, 1, dimidsp, &varid))) {

      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }
    if ((retval = nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }

    // FIXME we should have an internal representation of the data space in the
    // original type instead of transforming
    std::vector<float> values = ps->values();
    std::vector<int32_t> valuesInt;
    valuesInt.resize(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      valuesInt[i] = values.size();
    }

    nc_put_var(datagrpid, varid, valuesInt.data());
  } else if (ps->datatype == ParameterSpaceDimension::UINT32) {

    int dimidsp[1] = {dimid};
    if ((retval =
             nc_def_var(datagrpid, "values", NC_INT, 1, dimidsp, &varid))) {

      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }
    if ((retval = nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }

    // FIXME we should have an internal representation of the data space in the
    // original type instead of transforming
    std::vector<float> values = ps->values();
    std::vector<uint32_t> valuesInt;
    valuesInt.resize(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      valuesInt[i] = values.size();
    }

    nc_put_var(datagrpid, varid, valuesInt.data());
  }
  return true;
}

bool ParameterSpace::writeToNetCDF(std::string fileName) {
  int retval;
  int ncid;
  fileName = al::File::conformPathToOS(rootPath) + fileName;

  if ((retval = nc_create(fileName.c_str(), NC_CLOBBER | NC_NETCDF4, &ncid))) {
    return false;
  }
  int grpid;
  if ((retval = nc_def_grp(ncid, "internal_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }

  for (auto ps : dimensions) {
    if (ps->type == ParameterSpaceDimension::INTERNAL) {
      int datagrpid;
      if ((retval = nc_def_grp(grpid, ps->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, ps)) {
        return false;
      }
    }
  }

  if ((retval = nc_def_grp(ncid, "index_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  for (auto ps : dimensions) {
    if (ps->type == ParameterSpaceDimension::INDEX) {
      int datagrpid;
      if ((retval = nc_def_grp(grpid, ps->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, ps)) {
        return false;
      }
    }
  }

  if ((retval = nc_def_grp(ncid, "mapped_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  for (auto ps : dimensions) {
    if (ps->type == ParameterSpaceDimension::MAPPED) {
      int shuffle = 1;
      int deflate = 9;
      int datagrpid;
      int varid;
      int dimid;
      if ((retval = nc_def_grp(grpid, ps->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, ps)) {
        return false;
      }

      // Insert ids --------
      if ((retval = nc_def_dim(datagrpid, "id_dim", ps->size(), &dimid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }

      int dimidsp[1] = {dimid};
      if ((retval =
               nc_def_var(datagrpid, "ids", NC_STRING, 1, dimidsp, &varid))) {

        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }

      if ((retval =
               nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      auto ids = ps->ids();
      char **idArray = (char **)calloc(ids.size(), sizeof(char *));
      size_t start[1] = {0};
      size_t count[1] = {ids.size()};
      for (size_t i = 0; i < ids.size(); i++) {
        idArray[i] = (char *)calloc(ids[i].size(), sizeof(char));
        strncpy(idArray[i], ids[i].data(), ids[i].size());
      }
      if ((retval = nc_put_vara_string(datagrpid, varid, start, count,
                                       (const char **)idArray))) {
        std::cerr << nc_strerror(retval) << std::endl;

        for (size_t i = 0; i < ids.size(); i++) {
          free(idArray[i]);
        }
        free(idArray);
        return false;
      }

      for (size_t i = 0; i < ids.size(); i++) {
        free(idArray[i]);
      }
      free(idArray);
    }
  }

  if ((retval = nc_close(ncid))) {
    return false;
  }
  //  std::map<std::string, size_t> indeces;
  //  for (auto ps : mappedParameters) {
  //    indeces[ps->getName()] = 0;
  //  }
  //  for (auto ps : conditionParameters) {
  //    indeces[ps->getName()] = 0;
  //  }

  //  do {
  //    std::string newPath = generateRelativePath(indeces);
  //    std::stringstream ss(newPath);
  //    std::string item;
  //    std::vector<std::string> newPathComponents;
  //    while (std::getline(ss, item, AL_FILE_DELIMITER)) {
  //      newPathComponents.push_back(std::move(item));
  //    }

  //    auto newIt = newPathComponents.begin();

  //    std::string subPath;
  //    while (newIt != newPathComponents.end()) {
  //      subPath += *newIt + AL_FILE_DELIMITER_STR;
  //      std::cout << "Writing parameter space at " << subPath << std::endl;
  //      newIt++;
  //    }
  //  } while (!incrementIndeces(indeces));
  return true;
}

void ParameterSpace::registerChangeCallback(
    std::function<void(float, ParameterSpaceDimension *)> changeCallback) {
  mChangeCallback = changeCallback;
}

void ParameterSpace::updateParameterSpace(float oldValue,
                                          ParameterSpaceDimension *ps) {
  if (mSpecialDirs.size() == 0) {
    return; // No need to check
  }
  bool isFileSystemParam = false;
  std::map<std::string, size_t> indeces;
  for (auto dimension : dimensions) {
    if (dimension->type == ParameterSpaceDimension::INDEX ||
        dimension->type == ParameterSpaceDimension::MAPPED) {
      indeces[dimension->getName()] = dimension->getCurrentIndex();
      if (dimension.get() == ps) {
        isFileSystemParam = true;
        indeces[dimension->getName()] =
            dimension->getIndexForValue(ps->getCurrentValue());
      }
    }
  }

  if (isFileSystemParam) {
    std::string newPath = generateRelativeRunPath(indeces);
    std::stringstream ss(newPath);
    std::string item;
    std::vector<std::string> newPathComponents;
    while (std::getline(ss, item, AL_FILE_DELIMITER)) {
      newPathComponents.push_back(std::move(item));
    }

    auto path = currentRunPath();
    std::stringstream ss2(path);
    std::vector<std::string> oldPathComponents;
    while (std::getline(ss2, item, AL_FILE_DELIMITER)) {
      oldPathComponents.push_back(std::move(item));
    }

    auto oldIt = oldPathComponents.begin();
    auto newIt = newPathComponents.begin();

    std::string oldSubPath;
    std::string subPath;
    bool needsRefresh = false;

    std::vector<std::shared_ptr<ParameterSpaceDimension>> newDimensions;
    while (oldIt != oldPathComponents.end() &&
           newIt != newPathComponents.end()) {
      if (*oldIt != *newIt) {
        subPath += *newIt + AL_FILE_DELIMITER_STR;
        oldSubPath += *oldIt + AL_FILE_DELIMITER_STR;
        if (mSpecialDirs.find(subPath) != mSpecialDirs.end() ||
            mSpecialDirs.find(oldSubPath) != mSpecialDirs.end()) {
          needsRefresh = true;
          break;
        }
      }
      oldIt++;
      newIt++;
    }

    if (needsRefresh) {
      // For now, recreate whole paramter space, this could be optimized in the
      // future through caching
      std::vector<std::shared_ptr<ParameterSpaceDimension>> newDimensions;
      std::string filename =
          al::File::conformPathToOS(rootPath) + "parameter_space.nc";
      if (!readDimensionsInNetCDFFile(filename, newDimensions)) {
        std::cerr << "ERROR reading root parameter space" << std::endl;
      }

      for (auto newDim : newDimensions) {
        registerDimension(newDim);
      }
      newDimensions.clear();
      // FIXME remove dimensions in ParameterSpace that are no longer used

      subPath.clear();
      newIt = newPathComponents.begin();
      while (newIt != newPathComponents.end()) {
        subPath += *newIt + AL_FILE_DELIMITER_STR;
        if (mSpecialDirs.find(subPath) != mSpecialDirs.end() &&
            al::File::exists(al::File::conformPathToOS(rootPath) + subPath +
                             mSpecialDirs[subPath])) {
          std::cout << "Loading parameter space at " << subPath << std::endl;
          readDimensionsInNetCDFFile(al::File::conformPathToOS(rootPath) +
                                         subPath + mSpecialDirs[subPath],
                                     newDimensions);
        }
        newIt++;
      }

      for (auto newDim : newDimensions) {
        registerDimension(newDim);
      }
    }
  }
}
