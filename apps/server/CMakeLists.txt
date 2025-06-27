find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(bim-server main.cpp)
target_link_libraries(
  bim-server
  PRIVATE
  bim_server
  iscool_json
  Boost::program_options
)
post_build_strip(bim-server)
