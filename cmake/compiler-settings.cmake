set(CMAKE_CXX_STANDARD 23)
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

  option(BIM_GENERIC_HARDWARE
    "Compile for a generic hardware architecture (compilers's default)."
    OFF
  )
  message(STATUS "Targeting Generic hardware: ${BIM_GENERIC_HARDWARE}")

  if (NOT BIM_BUILDING_FOR_ANDROID AND NOT BIM_GENERIC_HARDWARE)
    add_compile_options(-march=x86-64-v3)
  endif()

  # F-Droid requires identical binaries between its build and the
  # reference APK, thus we need a reproducible build for Android
  # release builds. We are going to drop the build ID in this case as
  # it is a cause of different binaries.
  if(BIM_BUILDING_FOR_ANDROID AND (CMAKE_BUILD_TYPE STREQUAL Release))
    message(STATUS "Disabling build ID.")
    add_link_options("LINKER:--build-id=none")
  endif()

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Those warnings are bogus.
    add_compile_options(
      -Wno-stringop-overflow
      -Wno-alloc-size-larger-than
    )
  endif()

  option(BIM_ADDRESS_SANITIZER "Compile with AddressSanitizer enabled" OFF)

  if(BIM_ADDRESS_SANITIZER)
    add_compile_options(
      -fsanitize=address -fsanitize=undefined
    )
    add_link_options(
      -fsanitize=address
      -fsanitize=undefined
      -fno-sanitize-recover=all
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

  # Since the dependency scripts always enable LTO to build the
  # dependencies, the compiler may decide to use LTO at link time even
  # if not asked to (e.g. debug build). Consequently, we force it not
  # to use LTO if it is not enabled at the app level.
  if (NOT CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    message(STATUS "Disabling LTO explicitly.")
    add_link_options(-fno-lto)
  endif()
endif()

if(NOT post_build_strip_defined)
  function(post_build_strip target)
  endfunction()
endif()

unset(post_build_strip_defined)

message(STATUS "CXX flags are: ${CMAKE_CXX_FLAGS}")
