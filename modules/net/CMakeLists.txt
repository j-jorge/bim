add_library(bm_net STATIC
  main/src/bm/net/exchange/authentication_exchange.cpp
)

target_include_directories(bm_net PUBLIC main/include)
target_link_libraries(bm_net PUBLIC iscool_net iscool_random)
