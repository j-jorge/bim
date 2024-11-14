find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(bim-player main.cpp)
target_link_libraries(bim-player
  PRIVATE
  bim_game
  Boost::program_options
)
