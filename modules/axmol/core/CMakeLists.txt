find_package(axmol 1.0.0 REQUIRED)

add_library(bim_axmol_core
  STATIC
  main/src/bim/axmol/bounding_box_on_screen.cpp
  main/src/bim/axmol/colour_chart.cpp
  main/src/bim/axmol/displayed.cpp
  main/src/bim/axmol/find_child_by_path.cpp
  main/src/bim/axmol/ref_ptr.cpp
)

target_include_directories(bim_axmol_core PUBLIC main/include)

target_link_libraries(
  bim_axmol_core
  PUBLIC
  axmol::axmol
  iscool_log
)

add_executable(axmol-core-tests
  tests/src/bim/axmol/colour_chart.cpp
  tests/src/bim/axmol/displayed.cpp
  tests/src/bim/axmol/find_child_by_path.cpp
  tests/src/bim/axmol/ref_ptr.cpp
)
target_link_libraries(
  axmol-core-tests
  PRIVATE
  bim_axmol_core
  GTest::gtest
  bim_gtest_main
)
