find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(bim main.cpp)
target_link_libraries(bim
  PRIVATE
  bim_app
  iscool_strings
  Boost::program_options
)
