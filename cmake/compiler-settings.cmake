set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  add_compile_options(
    -Wall
    -pedantic
    -Werror
    -fvisibility=hidden
    "-fmacro-prefix-map=${BIM_PROJECT_ROOT}/=./"
  )

  if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13))
    # This error is reported on some calls to std::vector::push_back()
    # for which I cannot find any problem, so either I do not
    # understand or it is a false positive.
    add_compile_options(-Wno-stringop-overflow)
  endif()

  option(BIM_ADDRESS_SANITIZER "Compile with AddressSanitizer enabled" OFF)

  if(BIM_ADDRESS_SANITIZER)
    add_compile_options(
      -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
    )
    add_link_options(
      -fsanitize=address
      -fsanitize=undefined
      -static-libasan
      -static-libubsan
    )
  endif()

  option(BIM_THREAD_SANITIZER "Compile with ThreadSanitizer enabled" OFF)

  if(BIM_THREAD_SANITIZER)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread -static-libtsan)
  endif()

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    # Emit debug info in release builds too, such that we can have
    # symbols in the debugger.
    add_compile_options(-g)
  endif()
endif()

