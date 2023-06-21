find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(bim-console
  src/bim/app/console/application.cpp
  src/bim/app/console/display.cpp
  src/bim/app/console/inputs.cpp
  src/bim/app/console/offline_game.cpp
  src/bim/app/console/online_game.cpp
  src/bim/app/console/scoped_terminal_setup.cpp
  main.cpp
)
target_include_directories(bim-console PRIVATE include)
target_link_libraries(bim-console
  PRIVATE
  bim_net
  Boost::program_options
)
