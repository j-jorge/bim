find_package(axmol 1.0.0 REQUIRED)

add_library(bim_axmol_files
  STATIC
  main/src/bim/axmol/files/bridge.cpp
)

target_include_directories(bim_axmol_files PUBLIC main/include)

target_link_libraries(
  bim_axmol_files
  PUBLIC
  axmol::axmol
  iscool_files
)
