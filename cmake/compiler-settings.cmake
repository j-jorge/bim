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
    -fno-omit-frame-pointer
  )

  # F-Droid requires identical binaries between its build and the
  # reference APK, thus we need a reproducible build for Android
  # release builds. We are going to drop the build ID in this case as
  # it is a cause of different binaries.
  if(BIM_BUILDING_FOR_ANDROID AND (CMAKE_BUILD_TYPE STREQUAL Release))
    message(STATUS "Disabling build ID.")
    add_link_options("LINKER:--build-id=none")
  endif()

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
      -fsanitize=address -fsanitize=undefined
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
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread -static-libtsan)
  endif()

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    # Emit debug info in release builds too, such that we can have
    # symbols in the debugger.
    add_compile_options(-g)

    # The .gnu_debuglink section is different between what is built on
    # GitHub and the output of F-Droid, so we can't use this section
    # if we want reproducible builds.
    if (NOT BIM_BUILDING_FOR_ANDROID)
      set(post_build_strip_defined ON)
      function(post_build_strip target)
        add_custom_command(
          TARGET ${target}
          POST_BUILD
          COMMAND
            ${CMAKE_OBJCOPY}
            --only-keep-debug
            $<TARGET_FILE:${target}>
            $<TARGET_FILE:${target}>.dbg
          COMMAND
            ${CMAKE_STRIP} --strip-debug --strip-unneeded
            $<TARGET_FILE:${target}>
          COMMAND
            ${CMAKE_OBJCOPY}
            --add-gnu-debuglink=$<TARGET_FILE:${target}>.dbg
            $<TARGET_FILE:${target}>
        )
      endfunction()
    endif()

    set(use_mold_default OFF)
  else()
    try_compile(
      use_mold_default
      SOURCE_FROM_CONTENT main.cpp "int main(){return 0;}"
      LINK_OPTIONS -fuse-ld=mold
    )
  endif()

  option(BIM_USE_MOLD "Link with mold linker" ${use_mold_default})

  if(BIM_USE_MOLD)
    message(STATUS "Using mold linker.")
    add_link_options(-fuse-ld=mold)
  else()
    message(STATUS "Using default linker.")
  endif()
endif()

if(NOT post_build_strip_defined)
  function(post_build_strip target)
  endfunction()
endif()

unset(post_build_strip_defined)
