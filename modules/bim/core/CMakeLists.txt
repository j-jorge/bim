configure_file(version.hpp.in include/bim/version.hpp @ONLY)

add_library(bim_core INTERFACE)
target_include_directories(
  bim_core
  INTERFACE
  main/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)
