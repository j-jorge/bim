add_library(bim_axmol_action
  STATIC
  main/src/bim/axmol/action/dynamic_factory.cpp
  main/src/bim/axmol/action/register_actions.cpp

  main/src/bim/axmol/action/factory/scale.cpp
  main/src/bim/axmol/action/factory/spawn.cpp
  main/src/bim/axmol/action/factory/tint.cpp
  main/src/bim/axmol/action/factory/wrap_in_easing_function.cpp
)

target_include_directories(bim_axmol_action PUBLIC main/include)

target_link_libraries(
  bim_axmol_action
  PUBLIC
  bim_axmol_core
  bim_core
)
