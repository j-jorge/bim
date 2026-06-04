if (NOT BIM_BUILD_SERVER)
  return()
endif()

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(cpptrace REQUIRED)
find_package(CURL REQUIRED)

add_executable(bim-stack-dump main_stack_dump.cpp)
target_link_libraries(
  bim-stack-dump
  PRIVATE
  cpptrace::cpptrace
)
post_build_strip(bim-stack-dump)

add_executable(
  bim-server
  http_worker.cpp
  main_server.cpp
)
target_link_libraries(
  bim-server
  PRIVATE
  bim_server
  iscool_http
  iscool_json
  Boost::program_options
  cpptrace::cpptrace
  CURL::libcurl
)
post_build_strip(bim-server)
add_dependencies(bim-server bim-stack-dump)
