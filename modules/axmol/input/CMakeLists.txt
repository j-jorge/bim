add_library(bim_axmol_input
  STATIC
  main/src/bim/axmol/input/flow.cpp
  main/src/bim/axmol/input/key_observer.cpp
  main/src/bim/axmol/input/node.cpp
  main/src/bim/axmol/input/node_reference.cpp
  main/src/bim/axmol/input/template_instantiations.cpp
  main/src/bim/axmol/input/touch_observer.cpp
  main/src/bim/axmol/input/tree.cpp

  main/src/bim/axmol/input/observer/axmol_node_touch_observer.cpp
  main/src/bim/axmol/input/observer/key_sink.cpp
  main/src/bim/axmol/input/observer/single_key_observer.cpp
  main/src/bim/axmol/input/observer/tap_observer.cpp
  main/src/bim/axmol/input/observer/touch_sink.cpp
)

target_include_directories(bim_axmol_input PUBLIC main/include)

target_link_libraries(
  bim_axmol_input
  PUBLIC
  bim_axmol_core
  iscool_schedule
)

add_executable(axmol-input-tests
  tests/src/bim/axmol/input/backend_event_view.cpp
  tests/src/bim/axmol/input/node_key_observer.cpp
  tests/src/bim/axmol/input/node_touch_observer.cpp
)
target_link_libraries(
  axmol-input-tests
  PRIVATE
  bim_axmol_input
  GTest::gtest
  GTest::gtest_main
)
