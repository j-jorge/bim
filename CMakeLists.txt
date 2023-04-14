cmake_minimum_required(VERSION 3.24)

project(bombers-man)

set(CMAKE_CXX_STANDARD 20)

find_package(EnTT 3.11.1 REQUIRED)

add_library(bm_game
  STATIC
  game/src/bm/game/arena.cpp
  game/src/bm/game/contest.cpp
)
target_include_directories(bm_game PUBLIC ./game/include)

add_executable(bombers-man-console console/main.cpp)
target_link_libraries(bombers-man-console PRIVATE bm_game)
