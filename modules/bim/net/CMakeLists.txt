add_library(bim_net STATIC
  main/src/bim/net/contest_runner.cpp
  main/src/bim/net/session_handler.cpp

  main/src/bim/net/exchange/authentication_exchange.cpp
  main/src/bim/net/exchange/game_update_exchange.cpp
  main/src/bim/net/exchange/new_game_exchange.cpp

  main/src/bim/net/message/game_update_from_client.cpp
  main/src/bim/net/message/game_update_from_server.cpp
  main/src/bim/net/message/launch_game.cpp
  main/src/bim/net/message/player_action_serialization.cpp
)

target_include_directories(bim_net PUBLIC main/include)
target_link_libraries(
  bim_net
  PUBLIC
  bim_game
  iscool_http
  iscool_net
  iscool_random
)

add_executable(net-tests
  tests/src/bim/net/message/fuzzing.cpp
  tests/src/bim/net/message/game_update_from_client.cpp
  tests/src/bim/net/message/game_update_from_server.cpp
  tests/src/bim/net/message/launch_game.cpp
)
target_link_libraries(net-tests PRIVATE bim_net GTest::gtest bim_gtest_main)
