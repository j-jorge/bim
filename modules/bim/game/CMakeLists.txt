add_library(bim_game
  STATIC
  main/src/bim/game/arena.cpp
  main/src/bim/game/contest.cpp
  main/src/bim/game/contest_runner.cpp
  main/src/bim/game/input_archive.cpp
  main/src/bim/game/level_generation.cpp
  main/src/bim/game/output_archive.cpp
  main/src/bim/game/random_generator.cpp
  main/src/bim/game/tick_counter.cpp

  main/src/bim/game/component/player_action_kind.cpp

  main/src/bim/game/factory/bomb.cpp
  main/src/bim/game/factory/brick_wall.cpp
  main/src/bim/game/factory/flame.cpp

  main/src/bim/game/system/apply_player_action.cpp
  main/src/bim/game/system/remove_dead_objects.cpp
  main/src/bim/game/system/update_bombs.cpp
  main/src/bim/game/system/update_brick_walls.cpp
  main/src/bim/game/system/update_flames.cpp
)
target_include_directories(bim_game PUBLIC main/include)
target_link_libraries(bim_game PUBLIC bim_core EnTT::EnTT)

add_executable(game-tests
  tests/src/bim/game/archive.cpp
  tests/src/bim/game/arena.cpp
  tests/src/bim/game/level_generation.cpp

  tests/src/bim/game/system/update_bombs.cpp
  tests/src/bim/game/system/update_brick_walls.cpp
  tests/src/bim/game/system/update_flames.cpp
)
target_link_libraries(
  game-tests
  PRIVATE
  bim_game
  GTest::gtest
  GTest::gtest_main
)

