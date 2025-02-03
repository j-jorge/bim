find_package(axmol 1.0.0 REQUIRED)

add_library(bim_axmol_style
  STATIC
  main/src/bim/axmol/style/apply_bounds.cpp
  main/src/bim/axmol/style/apply_display.cpp
  main/src/bim/axmol/style/apply_size.cpp
  main/src/bim/axmol/style/bounds_property_flags.cpp
  main/src/bim/axmol/style/cache.cpp
  main/src/bim/axmol/style/display_property_flags.cpp
)

target_include_directories(bim_axmol_style PUBLIC main/include)

target_link_libraries(
  bim_axmol_style
  PUBLIC
  bim_axmol_core
  iscool_style
  axmol::axmol
)

add_executable(axmol-style-tests
  tests/src/bim/axmol/style/apply_display.cpp
)
target_link_libraries(
  axmol-style-tests
  PRIVATE
  bim_axmol_style
  GTest::gtest
  bim_gtest_main
)
