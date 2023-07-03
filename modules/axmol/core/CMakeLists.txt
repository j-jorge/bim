find_package(axmol 1.0.0 REQUIRED)

add_library(bim_axmol_core
  STATIC
  main/src/bim/axmol/colour_chart.cpp)

target_include_directories(bim_axmol_core PUBLIC main/include)

target_link_libraries(
  bim_axmol_core
  PUBLIC
  axmol::axmol
  iscool_log
)

add_executable(axmol-core-tests
  tests/src/bim/axmol/colour_chart.cpp
)
target_link_libraries(
  axmol-core-tests
  PRIVATE
  bim_axmol_core
  GTest::gtest
  GTest::gtest_main
)
run_when_built(axmol-core-tests)
