find_package(axmol 1.0.0 REQUIRED)

add_library(bim_axmol_audio
  STATIC
  main/src/bim/axmol/audio/mixer.cpp
)

target_include_directories(bim_axmol_audio PUBLIC main/include)

target_link_libraries(
  bim_axmol_audio
  PUBLIC
  iscool_audio
  axmol::axmol
)
