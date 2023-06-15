find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(bombers-man-console
  src/bm/app/console/application.cpp
  src/bm/app/console/display.cpp
  src/bm/app/console/inputs.cpp
  src/bm/app/console/offline_game.cpp
  src/bm/app/console/online_game.cpp
  src/bm/app/console/scoped_terminal_setup.cpp
  main.cpp
)
target_include_directories(bombers-man-console PRIVATE include)
target_link_libraries(bombers-man-console
  PRIVATE
  bm_net
  Boost::program_options
)
