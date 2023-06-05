add_library(bm_net STATIC
  main/src/bm/net/exchange/authentication_exchange.cpp
  main/src/bm/net/exchange/new_game_exchange.cpp

  main/src/bm/net/message/game_update_from_client.cpp
  main/src/bm/net/message/game_update_from_server.cpp
)

target_include_directories(bm_net PUBLIC main/include)
target_link_libraries(bm_net PUBLIC bm_game iscool_net iscool_random)

add_executable(net-tests
  tests/src/bm/net/message/game_update_from_client.cpp
  tests/src/bm/net/message/game_update_from_server.cpp
)
target_link_libraries(net-tests PRIVATE bm_net GTest::gtest GTest::gtest_main)
run_when_built(net-tests)
