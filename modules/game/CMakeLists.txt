add_library(bm_game
  STATIC
  main/src/bm/game/arena.cpp
  main/src/bm/game/contest.cpp
  main/src/bm/game/level_generation.cpp
  main/src/bm/game/random_generator.cpp

  main/src/bm/game/factory/bomb.cpp
  main/src/bm/game/factory/brick_wall.cpp
  main/src/bm/game/factory/flame.cpp

  main/src/bm/game/system/apply_player_action.cpp
  main/src/bm/game/system/update_bombs.cpp
  main/src/bm/game/system/update_brick_walls.cpp
  main/src/bm/game/system/update_flames.cpp
)
target_include_directories(bm_game PUBLIC main/include)
target_link_libraries(bm_game PUBLIC EnTT::EnTT)

add_executable(game-tests
  tests/src/bm/game/arena.cpp
  tests/src/bm/game/level_generation.cpp

  tests/src/bm/game/system/update_bombs.cpp
  tests/src/bm/game/system/update_brick_walls.cpp
  tests/src/bm/game/system/update_flames.cpp
)
target_link_libraries(game-tests PRIVATE bm_game GTest::gtest GTest::gtest_main)
run_when_built(game-tests)
