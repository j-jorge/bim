cmake_minimum_required(VERSION 3.24)

project(bombers-man)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  add_compile_options(-Wall -pedantic)

  option(BOMB_ADDRESS_SANITIZER "Compile with AddressSanitizer enabled" OFF)

  if(BOMB_ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
  endif()
endif()

find_package(Boost 1.82.0 REQUIRED)
find_package(EnTT 3.11.1 REQUIRED)
find_package(GTest 1.13.0 REQUIRED)

add_library(bm_game
  STATIC
  game/main/src/bm/game/arena.cpp
  game/main/src/bm/game/contest.cpp
  game/main/src/bm/game/level_generation.cpp
  game/main/src/bm/game/random_generator.cpp

  game/main/src/bm/game/factory/flame.cpp

  game/main/src/bm/game/system/apply_player_action.cpp
  game/main/src/bm/game/system/update_bombs.cpp
)
target_include_directories(bm_game PUBLIC ./game/main/include)
target_link_libraries(bm_game PUBLIC EnTT::EnTT)

add_executable(game-tests
  ./game/tests/src/bm/game/arena.cpp
  ./game/tests/src/bm/game/level_generation.cpp

  ./game/tests/src/bm/game/system/update_bombs.cpp
)
target_link_libraries(game-tests PRIVATE bm_game GTest::gtest GTest::gtest_main)

add_executable(bombers-man-console console/main.cpp)
target_link_libraries(bombers-man-console PRIVATE bm_game)
