find_package(axmol 1.0.0 REQUIRED)

add_library(bim_axmol_display
  STATIC
  main/src/bim/axmol/display/device_scale.cpp
  main/src/bim/axmol/display/device_size.cpp
  main/src/bim/axmol/display/main_view.cpp
)

target_include_directories(bim_axmol_display PUBLIC main/include)

target_link_libraries(
  bim_axmol_display
  PUBLIC
  axmol::axmol
)
