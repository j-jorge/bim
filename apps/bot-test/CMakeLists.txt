find_package(Boost REQUIRED COMPONENTS program_options)
find_library(fmt NAMES fmt fmtd REQUIRED)

add_executable(bim-bot-test main.cpp)
target_link_libraries(bim-bot-test
  PRIVATE
  bim_game
  Boost::program_options
  ${fmt}
)
