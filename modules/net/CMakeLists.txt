add_library(bm_net STATIC
  main/src/bm/net/exchange/all.cpp
)

target_include_directories(bm_net PUBLIC main/include)
target_link_libraries(bm_net PUBLIC iscool_net)
