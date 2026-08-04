add_library(bim_business
  STATIC
  main/src/bim/business/post.cpp
  main/src/bim/business/request_headers.cpp
)
target_include_directories(bim_business PUBLIC main/include)
target_link_libraries(bim_business PUBLIC iscool_http)
