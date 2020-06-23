cmake_minimum_required(VERSION 3.8)

project(TINC)

set(TINC_SRC

    ${CMAKE_CURRENT_LIST_DIR}/src/AtomRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ComputationChain.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/CppProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DataScript.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ParameterSpace.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ParameterSpaceDimension.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ParameterSpaceNode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Processor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ProcessorAsync.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ProcessorServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VASPReader.cpp
  )

set(TINC_INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include)

set(TINC_HEADERS
    ${TINC_INCLUDE_PATH}/tinc/AtomRenderer.hpp
    ${TINC_INCLUDE_PATH}/tinc/BufferManager.hpp
    ${TINC_INCLUDE_PATH}/tinc/ComputationChain.hpp
    ${TINC_INCLUDE_PATH}/tinc/CppProcessor.hpp
    ${TINC_INCLUDE_PATH}/tinc/DataScript.hpp
    ${TINC_INCLUDE_PATH}/tinc/DeferredComputation.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/ImageDiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/JsonDiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/NetCDFDiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpace.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpaceDimension.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpaceNode.hpp
    ${TINC_INCLUDE_PATH}/tinc/Processor.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorAsync.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorServer.hpp
    ${TINC_INCLUDE_PATH}/tinc/PeriodicTask.hpp
    ${TINC_INCLUDE_PATH}/tinc/VASPReader.hpp
)

add_library(tinc ${TINC_SRC} ${TINC_HEADERS})

# binaries are put into the ./bin directory by default
set_target_properties(tinc PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
)

##### Binary dependencies
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/external/usr)
message(" ********** ${CMAKE_CURRENT_LIST_DIR}/external/usr")
FIND_PACKAGE(HDF5 COMPONENTS C HL REQUIRED)
FIND_LIBRARY(NETCDF_LIBRARY netCDF
  PATHS
  ${CMAKE_CURRENT_LIST_DIR}/external/usr/lib
  )

if(HDF5_FOUND)
  message("Using HDF5 root: ${HDF5_LIBRARIES}")
  target_compile_definitions(tinc PUBLIC -DTINC_HAS_HDF5)
  target_link_libraries(tinc PUBLIC ${HDF5_LIBRARIES})
  target_link_directories(tinc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external/usr/lib)
else()

endif(HDF5_FOUND)

if(NETCDF_LIBRARY AND HDF5_FOUND)
  message("Using NetCDF: ${NETCDF_LIBRARY}")
  target_compile_definitions(tinc PUBLIC -DTINC_HAS_NETCDF)
  target_link_libraries(tinc PUBLIC ${NETCDF_LIBRARY})
  target_link_directories(tinc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external/usr/lib)
else()

endif(NETCDF_LIBRARY AND HDF5_FOUND)

##### In tree build dependencies
include(buildDependencies.cmake)

##### Examples
add_subdirectory(examples)
