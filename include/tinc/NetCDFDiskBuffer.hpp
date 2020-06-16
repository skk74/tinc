#ifndef NETCDFDISKBUFFER_HPP
#define NETCDFDISKBUFFER_HPP

#include "tinc/DiskBuffer.hpp"

#ifdef TINC_HAS_NETCDF
#include <netcdf.h>
#endif

namespace tinc {

struct NC_Dimension {
  int dataType;
  /*
  #define NC_NAT          0
  #define NC_BYTE         1
  #define NC_CHAR         2
  #define NC_SHORT        3
  #define NC_INT          4
  #define NC_LONG         NC_INT
  #define NC_FLOAT        5
  #define NC_DOUBLE       6
  #define NC_UBYTE        7
  #define NC_USHORT       8
  #define NC_UINT         9
  #define NC_INT64        10
  #define NC_UINT64       11
  #define NC_STRING       12
  */
};

class NetCDFDiskBufferDouble : public DiskBuffer<std::vector<double>> {
public:
  NetCDFDiskBufferDouble(std::string name, std::string fileName = "",
                         std::string path = "", uint16_t size = 2)
      : DiskBuffer<std::vector<double>>(name, fileName, path, size) {
#ifndef TINC_HAS_NETCDF
    std::cerr << "ERROR: NetCDFDiskBufferDouble built wihtout NetCDF support"
              << std::endl;
    assert(0 == 1);
#endif
  }

  bool updateData(std::string filename) {
    if (filename.size() > 0) {
      m_fileName = filename;
    }

    auto buffer = getWritable();

    int ncid, retval;

#ifdef TINC_HAS_NETCDF
    /* Open the file. NC_NOWRITE tells netCDF we want read-only access
     * to the file.*/
    if ((retval = nc_open(filename.c_str(), NC_NOWRITE, &ncid))) {
      return false;
    }
    int varid;
    if ((retval = nc_inq_varid(ncid, "data", &varid))) {
      return false;
    }

    nc_type xtypep;
    char name[32];
    int ndimsp;
    int dimidsp[32];
    int *nattsp = nullptr;
    if ((retval = nc_inq_var(ncid, varid, name, &xtypep, &ndimsp, dimidsp,
                             nattsp))) {
      return false;
    }

    size_t lenp;
    if ((retval = nc_inq_dimlen(ncid, dimidsp[0], &lenp))) {
      return false;
    }
    buffer->resize(lenp);

    /* Read the data. */
    if ((retval = nc_get_var_double(ncid, varid, buffer->data()))) {
      return false;
    }

    //    /* Check the data. */
    //    for (x = 0; x < NX; x++)
    //      for (y = 0; y < NY; y++)
    //        if (data_in[x][y] != x * NY + y)
    //          return ERRCODE;

    /* Close the file, freeing all resources. */
    if ((retval = nc_close(ncid))) {
      return false;
    }
#endif

    BufferManager<std::vector<double>>::doneWriting(buffer);
    return true;
  }

protected:
  virtual bool parseFile(std::ifstream &file,
                         std::shared_ptr<std::vector<double>> newData) {

    return true;
  }
};

} // namespace tinc

#endif // NETCDFDISKBUFFER_HPP
