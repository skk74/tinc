cmake_minimum_required(VERSION 3.8)

project(TINC)

set(TINC_SRC

    ${CMAKE_CURRENT_LIST_DIR}/src/AtomRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ComputationChain.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/CppProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DataScript.cpp
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
    ${TINC_INCLUDE_PATH}/tinc/ComputationChain.hpp
    ${TINC_INCLUDE_PATH}/tinc/CppProcessor.hpp
    ${TINC_INCLUDE_PATH}/tinc/DataScript.hpp
    ${TINC_INCLUDE_PATH}/tinc/BufferManager.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/ImageDiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/JsonDiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/DeferredComputation.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpaceDimension.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpaceNode.hpp
    ${TINC_INCLUDE_PATH}/tinc/Processor.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorAsync.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorServer.hpp
    ${TINC_INCLUDE_PATH}/tinc/PeriodicTask.hpp
    ${TINC_INCLUDE_PATH}/tinc/VASPReader.hpp
)

add_library(tinc ${TINC_SRC} ${TINC_HEADERS})

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


# binaries are put into the ./bin directory by default
set_target_properties(tinc PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
)

add_subdirectory(examples)
