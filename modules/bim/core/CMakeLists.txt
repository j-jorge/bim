configure_file(version.hpp.in include/bim/version.hpp @ONLY)

add_library(bim_core INTERFACE)
target_include_directories(
  bim_core
  INTERFACE
  main/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_library(bim_gtest_main
  tests/src/gtest-main/gtest_main.cpp
)
target_link_libraries(
  bim_gtest_main
  PRIVATE
  GTest::gtest
  iscool_log
)
