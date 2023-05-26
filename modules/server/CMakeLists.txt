add_library(bm_server
  STATIC
  main/src/bm/server/server.cpp
  main/src/bm/server/service/authentication_service.cpp
  main/src/bm/server/service/game_info.cpp
  main/src/bm/server/service/game_service.cpp
  main/src/bm/server/service/matchmaking_service.cpp
)

target_include_directories(bm_server PUBLIC main/include)
target_link_libraries(bm_server PUBLIC bm_net)

add_executable(server-tests
  tests/src/bm/server/authentication.cpp
  tests/src/bm/server/game.cpp
  tests/src/bm/server/new_game.cpp

  tests/src/bm/server/service/game_info.cpp
  tests/src/bm/server/service/game_service.cpp

  tests/src/bm/server/tests/fake_scheduler.cpp
)
target_include_directories(server-tests PRIVATE tests/include)
target_link_libraries(
  server-tests
  PRIVATE
  bm_server
  GTest::gtest
  GTest::gtest_main
)
run_when_built(server-tests)
