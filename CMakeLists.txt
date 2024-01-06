cmake_minimum_required(VERSION 3.24)

project(bim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  add_compile_options(-Wall -pedantic)

  option(BOMB_ADDRESS_SANITIZER "Compile with AddressSanitizer enabled" OFF)

  if(BOMB_ADDRESS_SANITIZER)
    add_compile_options(
      -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
    )
    add_link_options(-fsanitize=address -fsanitize=undefined)
  endif()

  option(BOMB_THREAD_SANITIZER "Compile with ThreadSanitizer enabled" OFF)

  if(BOMB_THREAD_SANITIZER)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
  endif()
endif()

find_package(axmol 2.0.0 REQUIRED)
find_package(Boost 1.82.0 REQUIRED)
find_package(EnTT 3.11.1 REQUIRED)
find_package(GTest 1.13.0 REQUIRED)
find_package(iscool-core 1.9.0 REQUIRED)

include(./cmake/run-when-built.cmake)

add_subdirectory(modules/bim/core)
add_subdirectory(modules/bim/game)
add_subdirectory(modules/bim/net)
add_subdirectory(modules/bim/server)

add_subdirectory(modules/axmol/core)
add_subdirectory(modules/axmol/audio)
add_subdirectory(modules/axmol/display)
add_subdirectory(modules/axmol/files)
add_subdirectory(modules/axmol/input)
add_subdirectory(modules/axmol/schedule)
add_subdirectory(modules/axmol/style)
add_subdirectory(modules/axmol/widget)

add_subdirectory(assets)

add_subdirectory(apps/axmol)
add_subdirectory(apps/linux)
add_subdirectory(apps/console)
add_subdirectory(apps/server)
