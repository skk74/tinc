#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "tinc/NetCDFDiskBuffer.hpp"

using namespace al;
using namespace tinc;

struct MyApp : public App {

  NetCDFDiskBufferDouble buffer{"NetCDFBuffer", "test.cdf"};
  VAOMesh m;

  void onInit() override {
    m.primitive(Mesh::LINE_STRIP);
    data.push_back(rnd::normal());
    data.push_back(rnd::normal());
  }

  void onAnimate(double dt) override {

    if (buffer.newDataAvailable()) {
      auto bufferData = buffer.get();
      m.reset();
      bool switcher = true;
      double valueCache;
      for (auto value : *bufferData) {
        if (switcher) {
          valueCache = value;
        } else {
          m.vertex(valueCache, value, -8);
        }
        switcher = !switcher;
      }
      m.update();
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.color(1);
    g.draw(m);
  }

  bool onKeyDown(const Keyboard &k) override {
    data.push_back(rnd::normal());
    data.push_back(rnd::normal());
    if (writeDataToDisk(data)) {
      buffer.updateData(buffer.getCurrentFileName());
    }
    return true;
  }

  bool writeDataToDisk(std::vector<double> &data) {
#define NDIMS 1
    int ncid, x_dimid, varid;
    int dimids[NDIMS];

    int retval;

    /* Create the file. The NC_CLOBBER parameter tells netCDF to
     * overwrite this file, if it already exists.*/
    if ((retval = nc_create(buffer.getCurrentFileName().c_str(),
                            NC_NETCDF4 | NC_CLOBBER, &ncid))) {
      return false;
    }

    /* Define the dimensions. NetCDF will hand back an ID for each. */
    if ((retval = nc_def_dim(ncid, "double", data.size(), &x_dimid))) {
      return false;
    }
    dimids[0] = x_dimid;

    /* Define the variable.*/
    if ((retval = nc_def_var(ncid, "data", NC_DOUBLE, NDIMS, dimids, &varid))) {
      return false;
    }

    /* End define mode. This tells netCDF we are done defining
     * metadata. */
    if ((retval = nc_enddef(ncid))) {
      return false;
    }

    /* Write the pretend data to the file. Although netCDF supports
     * reading and writing subsets of data, in this case we write all
     * the data in one operation. */
    if ((retval = nc_put_var_double(ncid, varid, data.data()))) {
      return false;
    }

    /* Close the file. This frees up any internal netCDF resources
     * associated with the file, and flushes any buffers. */
    if ((retval = nc_close(ncid))) {
      return false;
    }

    return true;
  }

  std::vector<double> data;
};

int main() {
  MyApp().start();
  return 0;
}
