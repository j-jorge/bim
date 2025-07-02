option(BIM_BUILD_SERVER
  "Whether the standalone server should be built or not."
  ON)

if (NOT BIM_BUILD_SERVER)
  return()
endif()

add_library(bim_server
  STATIC
  main/src/bim/server/config.cpp
  main/src/bim/server/server.cpp
  main/src/bim/server/service/authentication_service.cpp
  main/src/bim/server/service/contest_timeline_service.cpp
  main/src/bim/server/service/game_info.cpp
  main/src/bim/server/service/game_service.cpp
  main/src/bim/server/service/lobby_service.cpp
  main/src/bim/server/service/matchmaking_service.cpp
  main/src/bim/server/service/named_game_encounter_service.cpp
  main/src/bim/server/service/random_game_encounter_service.cpp
)

target_include_directories(bim_server PUBLIC main/include)
target_link_libraries(bim_server PUBLIC bim_net)

if (NOT BIM_BUILD_TESTS)
  return()
endif()

add_executable(server-tests
  tests/src/bim/server/authentication.cpp
  tests/src/bim/server/game_creation.cpp
  tests/src/bim/server/game_update.cpp
  tests/src/bim/server/player_disconnection.cpp
  tests/src/bim/server/new_named_game.cpp
  tests/src/bim/server/new_game_after_game_over.cpp

  tests/src/bim/server/service/game_info.cpp
  tests/src/bim/server/service/game_service.cpp

  tests/src/bim/server/tests/client_server_simulator.cpp
  tests/src/bim/server/tests/fake_scheduler.cpp
  tests/src/bim/server/tests/new_test_config.cpp
  tests/src/bim/server/tests/test_client.cpp
)
target_include_directories(server-tests PRIVATE tests/include)
target_link_libraries(
  server-tests
  PRIVATE
  bim_server
  GTest::gtest
  bim_gtest_main
)
