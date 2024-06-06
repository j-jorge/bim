add_library(bim_axmol_http
  STATIC
  main/src/bim/axmol/http/bridge.cpp
)

target_include_directories(bim_axmol_http PUBLIC main/include)

target_link_libraries(
  bim_axmol_http
  PUBLIC
  axmol::axmol
  iscool_http
)
