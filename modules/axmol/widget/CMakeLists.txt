find_package(axmol 1.0.0 REQUIRED)

add_library(bim_axmol_widget
  STATIC
  main/src/bim/axmol/widget/add_group_as_children.cpp
  main/src/bim/axmol/widget/apply_bounds.cpp
  main/src/bim/axmol/widget/apply_display.cpp
  main/src/bim/axmol/widget/dynamic_factory.cpp
  main/src/bim/axmol/widget/register_widgets.cpp

  main/src/bim/axmol/widget/factory/label.cpp
  main/src/bim/axmol/widget/factory/node.cpp
  main/src/bim/axmol/widget/factory/sprite.cpp
)

target_include_directories(bim_axmol_widget PUBLIC main/include)

target_link_libraries(
  bim_axmol_widget
  PUBLIC
  bim_axmol_style
  bim_axmol_core
  bim_core
  iscool_audio
)

add_executable(axmol-widget-tests
  tests/src/bim/axmol/widget/factory/node.cpp
)
target_link_libraries(
  axmol-widget-tests
  PRIVATE
  bim_axmol_widget
  GTest::gtest
  GTest::gtest_main
)
run_when_built(axmol-widget-tests)
