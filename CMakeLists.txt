cmake_minimum_required(VERSION 3.24)

project(bombers-man)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  add_compile_options(-Wall -pedantic)

  option(BOMB_ADDRESS_SANITIZER "Compile with AddressSanitizer enabled" OFF)

  if(BOMB_ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
  endif()
endif()

find_package(Boost 1.82.0 REQUIRED)
find_package(EnTT 3.11.1 REQUIRED)
find_package(GTest 1.13.0 REQUIRED)
find_package(iscool-core 1.8.0 REQUIRED)

add_subdirectory(modules/game)
add_subdirectory(modules/message)
add_subdirectory(modules/server)

add_subdirectory(apps/console)
