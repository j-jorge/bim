add_library(bm_server
  STATIC
  main/src/bm/server/server.cpp
)

target_include_directories(bm_server PUBLIC main/include)
target_link_libraries(bm_server PUBLIC bm_message)
