configure_file(version.hpp.in include/bim/version.hpp @ONLY)
configure_file(tracy.hpp.in include/bim/tracy.hpp @ONLY)

set(library_files "main/src/bim/template_instantiations.cpp")

if (BIM_ENABLE_TRACY)
  set(library_files ${library_files} "main/src/bim/new_delete.cpp")
endif()

add_library(bim_core STATIC ${library_files})
target_include_directories(
  bim_core
  PUBLIC
  main/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

if (BIM_ENABLE_TRACY)
  target_link_libraries(bim_core PUBLIC Tracy::TracyClient)
endif()

if (NOT BIM_BUILD_TESTS)
  return()
endif()

add_library(bim_gtest_main
  tests/src/gtest-main/gtest_main.cpp
)
target_link_libraries(
  bim_gtest_main
  PRIVATE
  GTest::gtest
  iscool_log
)

add_executable(core-tests
  tests/src/bim/core/bit_map.cpp
  tests/src/bim/core/table_2d.cpp
)
target_link_libraries(
  core-tests
  PRIVATE
  bim_core
  GTest::gtest
  bim_gtest_main
)
