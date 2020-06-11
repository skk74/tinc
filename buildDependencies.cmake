
# add allolib as a subdirectory to the project
add_subdirectory(external/allolib)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/external/al_ext)
  message("Buiding extensions in al_ext")
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/al_ext)
  get_target_property(AL_EXT_LIBRARIES al_ext AL_EXT_LIBRARIES)
  target_link_libraries(tinc PUBLIC ${AL_EXT_LIBRARIES})
endif()

# link allolib to project
target_link_libraries(tinc PUBLIC al)
target_include_directories(tinc PUBLIC ${TINC_INCLUDE_PATH})

# For some reason the includes from allolib are not being passed to tinc. this does it manually
get_target_property(AL_INCLUDE_DIRS al INCLUDE_DIRECTORIES)
target_include_directories(tinc PUBLIC ${AL_INCLUDE_DIRS})

set(TINC_DEPS_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/external/usr/")


# --------- EXTERNAL PROJECTS ----------

#include(ExternalProject)

# --------- HDF5 --------------
#ExternalProject_add(hdf5
#  #DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/external
#  #BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/external/HDF5-build
#  INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/external/usr/

#  GIT_REPOSITORY https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git
#  GIT_TAG hdf5-1_10_6
# GIT_SHALLOW true
# GIT_PROGRESS  true

# BUILD_ALWAYS true

# UPDATE_COMMAND ""
#  CMAKE_ARGS
#    -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}
#    -DHDF5_BUILD_HL_LIB=ON
#    -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON
#    -DZLIB_USE_EXTERNAL:BOOL=OFF
#    -DHDF5_BUILD_CPP_LIB:BOOL=OFF
#    -DBUILD_SHARED_LIBS:BOOL=OFF
#    -DHDF5_BUILD_TOOLS:BOOL=OFF
#    -DCMAKE_INSTALL_PREFIX:PATH=${TINC_DEPS_INSTALL_PREFIX}
#  INSTALL_DIR ${TINC_DEPS_INSTALL_PREFIX}
#)

# ------------------ NETCDF4 -------------
#ExternalProject_Add(netcdf
#  #DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/external
#  #BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/external/netcdf-build
#  INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/external/usr/
#  DEPENDS hdf5

#  GIT_REPOSITORY https://github.com/Unidata/netcdf-c.git
#  GIT_TAG v4.7.2
#  GIT_SHALLOW true
#  GIT_PROGRESS  true

##  BUILD_ALWAYS true
#  CONFIGURE_COMMAND CPPFLAGS=-I${CMAKE_CURRENT_LIST_DIR}/external/usr/include LDFLAGS=-L${CMAKE_CURRENT_LIST_DIR}/external/usr/lib <SOURCE_DIR>/configure --enable-netcdf-4 --with-hdf5=${TINC_DEPS_INSTALL_PREFIX} --prefix ${TINC_DEPS_INSTALL_PREFIX} --enable-shared --disable-static

#  INSTALL_COMMAND make install
#  INSTALL_DIR "${TINC_DEPS_INSTALL_PREFIX}"
#)

#find_library(NETCDF_LIBRARY netcdf
#  PATHS ${TINC_DEPS_INSTALL_PREFIX}/lib)

#message("USING NETCDF:${NETCDF_LIBRARY}")

# --------- NETCDF4 CPP --------------
#ExternalProject_add(netcdf-cxx4
#  #DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/external
#  #BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/external/HDF5-build
#  INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/external/usr/

#  DEPENDS netcdf

#  GIT_REPOSITORY https://github.com/Unidata/netcdf-cxx4.git
#  GIT_TAG v4.3.1
# GIT_SHALLOW true
# GIT_PROGRESS  true

## BUILD_ALWAYS true

# UPDATE_COMMAND ""
#  CMAKE_ARGS
#    -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}
#    -DnetCDF_LIBRARIES=${NETCDF_LIBRARY}
#    -DCMAKE_PREFIX_PATH=${TINC_DEPS_INSTALL_PREFIX}
#    #-DnetCDF_INCLUDE_DIR=${TINC_DEPS_INSTALL_PREFIX}
##    -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON
##    -DZLIB_USE_EXTERNAL:BOOL=OFF
#    -DHDF5_ROOT=${TINC_DEPS_INSTALL_PREFIX}
#    -DBUILD_SHARED_LIBS:BOOL=ON
#    -NCXX_ENABLE_TESTS:BOOL=OFF
##    -DCMAKE_INSTALL_PREFIX:PATH=${TINC_DEPS_INSTALL_PREFIX}
#  INSTALL_DIR "${TINC_DEPS_INSTALL_PREFIX}"
#)

#add_dependencies(tinc netcdf-cxx4)


#find_library(NETCDF_LIBRARY netcdf
#  PATHS ${TINC_DEPS_INSTALL_PREFIX}/lib)

#target_link_libraries(tinc PUBLIC ${NETCDF_LIBRARY} )
