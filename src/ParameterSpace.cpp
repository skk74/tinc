#include "tinc/ParameterSpace.hpp"

#ifdef TINC_HAS_HDF5
#include <netcdf.h>
#endif

#include <iostream>

using namespace tinc;

ParameterSpace::ParameterSpace() {}

void ParameterSpace::loadFromNetCDF(std::string ncFile) {
#ifdef TINC_HAS_NETCDF
  conditionParameters.clear();
  parameters.clear();
  mappedParameters.clear();

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

  if ((retval = nc_open(ncFile.c_str(), NC_NOWRITE | NC_SHARE, &ncid))) {
    std::cerr << "Error opening file: " << ncFile << std::endl;
    goto done;
  }

  // Get main group ids
  if (nc_inq_grp_ncid(ncid, "internal_states", &internal_state_grpid)) {
    std::cerr << "Error finding group 'internal_states' in " << ncFile
              << std::endl;
    goto done;
  }
  if (nc_inq_grp_ncid(ncid, "parameters", &parameters_grpid)) {
    std::cerr << "Error finding group 'parameters' in " << ncFile << std::endl;
    goto done;
  }
  if (nc_inq_grp_ncid(ncid, "conditions", &conditions_grpid)) {
    std::cerr << "Error finding group 'conditions' in " << ncFile << std::endl;
    goto done;
  }

  // Get subgroup ids
  if (nc_inq_grps(internal_state_grpid, &num_state_grps, state_grp_ids)) {
    goto done;
  }
  if (nc_inq_grps(parameters_grpid, &num_parameters, parameters_ids)) {
    goto done;
  }
  if (nc_inq_grps(conditions_grpid, &num_conditions, conditions_ids)) {
    goto done;
  }

  // Read internal states variable data
  for (int i = 0; i < num_state_grps; i++) {
    char groupName[32];
    // FIXME need to check size of name first
    if (nc_inq_grpname(state_grp_ids[i], groupName)) {
      goto done;
    }

    if ((retval = nc_inq_varid(state_grp_ids[i], "values", &varid))) {
      goto done;
    }
    if ((retval = nc_inq_var(state_grp_ids[i], varid, nullptr, &xtypep, &ndimsp,
                             dimidsp, nattsp))) {
      goto done;
    }
    if ((retval = nc_inq_dimlen(state_grp_ids[i], dimidsp[0], &lenp))) {
      goto done;
    }
    std::vector<int32_t> data;
    data.resize(lenp);
    if ((retval = nc_get_var(state_grp_ids[i], varid, data.data()))) {
      goto done;
    }

    std::shared_ptr<ParameterSpaceDimension> pdim =
        std::make_shared<ParameterSpaceDimension>(groupName);
    pdim->append(data.data(), data.size());
    pdim->conform();
    registerParameter(pdim);
    //    std::cout << "internal state " << i << ":" << groupName
    //              << " length: " << lenp << std::endl;
  }

  // Read conditions
  for (int i = 0; i < num_conditions; i++) {
    char conditionName[32];
    // FIXME need to check size of name first
    if (nc_inq_grpname(conditions_ids[i], conditionName)) {
      goto done;
    }
    if ((retval = nc_inq_varid(conditions_ids[i], "values", &varid))) {
      goto done;
    }

    if ((retval = nc_inq_var(conditions_ids[i], varid, nullptr, &xtypep,
                             &ndimsp, dimidsp, nattsp))) {
      goto done;
    }
    if ((retval = nc_inq_dimlen(conditions_ids[i], dimidsp[0], &lenp))) {
      goto done;
    }
    std::vector<float> data;
    data.resize(lenp);
    if ((retval = nc_get_var(conditions_ids[i], varid, data.data()))) {
      goto done;
    }

    std::shared_ptr<ParameterSpaceDimension> pdim =
        std::make_shared<ParameterSpaceDimension>(conditionName);
    pdim->append(data.data(), data.size());

    pdim->conform();
    registerCondition(pdim);
    //    std::cout << "condition " << i << ":" << conditionName
    //              << " length: " << lenp << std::endl;
  }

  // Process mapped parameters
  for (int i = 0; i < num_parameters; i++) {
    char parameterName[32];
    // FIXME need to check size of name first
    if (nc_inq_grpname(parameters_ids[i], parameterName)) {
      goto done;
    }
    if ((retval = nc_inq_varid(parameters_ids[i], "values", &varid))) {
      goto done;
    }

    if ((retval = nc_inq_var(parameters_ids[i], varid, nullptr, &xtypep,
                             &ndimsp, dimidsp, nattsp))) {
      goto done;
    }
    if ((retval = nc_inq_dimlen(parameters_ids[i], dimidsp[0], &lenp))) {
      goto done;
    }
    std::vector<float> data;
    data.resize(lenp);
    if ((retval = nc_get_var(parameters_ids[i], varid, data.data()))) {
      goto done;
    }
    // Now get ids
    if ((retval = nc_inq_varid(parameters_ids[i], "ids", &varid))) {
      goto done;
    }

    if ((retval = nc_inq_var(parameters_ids[i], varid, nullptr, &xtypep,
                             &ndimsp, dimidsp, nattsp))) {
      goto done;
    }
    if ((retval = nc_inq_dimlen(parameters_ids[i], dimidsp[0], &lenp))) {
      goto done;
    }
    char totalData[lenp * 80];
    std::vector<char *> idData;

    idData.resize(lenp);
    for (int i = 0; i < lenp; i++) {
      idData[i] = totalData + (i * 80);
    }
    if ((retval = nc_get_var_string(parameters_ids[i], varid, idData.data()))) {
      goto done;
    }

    std::shared_ptr<ParameterSpaceDimension> pdim =
        std::make_shared<ParameterSpaceDimension>(parameterName);
    pdim->append(data.data(), data.size());
    pdim->mIds.resize(lenp);
    for (int i = 0; i < lenp; i++) {
      pdim->mIds[i] = idData[i];
    }

    pdim->conform();
    registerMappedParameter(pdim);

    std::cout << "mapped parameter " << i << ":" << parameterName
              << " length: " << lenp << std::endl;
  }

  //  mParameterSpaces["time"]->append(timeSteps.data(), timeSteps.size());
  if ((retval = nc_close(ncid))) {
    goto done;
  }

#else
  std::cerr << "TINC built without NetCDF support. "
               "ParameterSpaceDimension::loadFromNetCDF() does not work."
            << std::endl;
#endif

done:
  return;
}
