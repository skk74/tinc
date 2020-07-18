#!/bin/sh
cd external

git clone --depth 1 --branch hdf5-1_12_0 https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git
cmake -S hdf5 -B hdf5_build -DBUILD_TESTING:BOOL=OFF -DHDF5_BUILD_EXAMPLES:BOOL=OFF -DHDF5_BUILD_HL_LIB:BOOL=ON -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON -DZLIB_USE_EXTERNAL:BOOL=OFF -DHDF5_BUILD_CPP_LIB:BOOL=OFF -DHDF5_BUILD_TOOLS:BOOL=OFF -DBUILD_STATIC:BOOL=ON
cmake --build hdf5_build -- -j7
cmake --install hdf5_build --prefix ./usr

git clone --depth 1 --branch v4.7.4 https://github.com/Unidata/netcdf-c.git
pushd netcdf-c
make clean
CPPFLAGS=-I`pwd`/../usr/include LDFLAGS=-L`pwd`/../usr/lib ./configure --enable-netcdf-4 --with-hdf5=`pwd`/../usr/share/cmake/hdf5  --prefix=`pwd`/../usr --enable-shared --disable-static
make -j7
make install
#cmake -S netcdf-c -B netcdf-c-build
popd


#git clone --depth 1 --branch v4.3.1 https://github.com/Unidata/netcdf-cxx4.git
#cmake -S netcdf-cxx4 -B netcdf-cxx4_build -DCMAKE_PREFIX_PATH=`pwd`/usr -DBUILD_TESTING:BOOL=OFF -DHDF5_BUILD_EXAMPLES:BOOL=OFF -DHDF5_BUILD_HL_LIB:BOOL=ON -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON -DZLIB_USE_EXTERNAL:BOOL=OFF -DHDF5_BUILD_CPP_LIB:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=ON -DHDF5_BUILD_TOOLS:BOOL=OFF
#cmake --build netcdf-cxx4_build -- -j7
#cmake --install netcdf-cxx4_build --prefix ./usr
