
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

