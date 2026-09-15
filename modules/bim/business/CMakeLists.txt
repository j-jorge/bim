configure_file(business_url.hpp.in include/bim/business/business_url.hpp @ONLY)

add_library(bim_business
  STATIC
  main/src/bim/business/post.cpp
  main/src/bim/business/request_headers.cpp
)
target_include_directories(
  bim_business
  PUBLIC
  main/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_libraries(bim_business PUBLIC iscool_http)
