find_package(axmol 1.0.0 REQUIRED)

add_library(bim_axmol_schedule
  STATIC
  main/src/bim/axmol/schedule/bridge.cpp
)

target_include_directories(bim_axmol_schedule PUBLIC main/include)

target_link_libraries(
  bim_axmol_schedule
  PUBLIC
  bim_axmol_core
  iscool_schedule
  fmt::fmt
)
