set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  add_compile_options(-Wall -pedantic)

  option(BIM_ADDRESS_SANITIZER "Compile with AddressSanitizer enabled" OFF)

  if(BIM_ADDRESS_SANITIZER)
    add_compile_options(
      -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
    )
    add_link_options(-fsanitize=address -fsanitize=undefined)
  endif()

  option(BIM_THREAD_SANITIZER "Compile with ThreadSanitizer enabled" OFF)

  if(BIM_THREAD_SANITIZER)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
  endif()
endif()

