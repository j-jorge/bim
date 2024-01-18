add_library(bim_axmol_jni
  STATIC
  main/src/bim/axmol/jni/bridge.cpp
)

target_include_directories(bim_axmol_jni PUBLIC main/include)

target_link_libraries(
  bim_axmol_jni
  PUBLIC
  axmol::axmol
  iscool_jni
)
