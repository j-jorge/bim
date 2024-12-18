cmake_minimum_required(VERSION 3.22)

set(project_version 4)

set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_LIST_DIR}/cmake"
  ${CMAKE_MODULE_PATH}
)
include(bim-configuration)

if(BIM_TARGET STREQUAL "android")
  project(bim
    VERSION ${project_version}
    LANGUAGES NONE
  )

  # This is an invocation of the current script for each Android
  # architecture.
  include(./CMakeLists-android.txt)
elseif(BIM_TARGET STREQUAL "assets")
  project(bim LANGUAGES NONE)

  # Just generate the assets, this is needed for Android builds and it
  # uses tools from the host.
  add_subdirectory(assets)
else()
  # By default CMAKE_FIND_ROOT_PATH is configured to point to the host
  # directory. If the host is the same than the target system, then
  # the target prefix can be used to find the libraries too.
  if (BIM_TARGET STREQUAL "linux")
    list(PREPEND CMAKE_FIND_ROOT_PATH "${BIM_TARGET_PREFIX}")
  endif()

  # Everything here is executed on the environment of the target
  # platform.
  project(bim
    VERSION ${project_version}
    LANGUAGES CXX
  )
  include(./CMakeLists-common.txt)
endif()
