set(BIM_GENERATED_ASSETS_DIR
  "${CMAKE_BINARY_DIR}/assets/generated"
  CACHE
  PATH
  "Path to the directory where the generated assets will be produced."
)

if(NOT DEFINED BIM_BUILDING_FOR_ANDROID)
  # When the Android build is configured this variable is set by the
  # calling CMake script. This enables us to include the correct
  # modules for this platform. Note that we cannot check
  # CMAKE_SYSTEM_NAME for equality with "Android" because this test is
  # also true when building the Linux or console apps from an Android
  # host.
  set(BIM_BUILDING_FOR_ANDROID FALSE)
endif()

option(BIM_BUILD_TESTS
  "Whether test programs should be compiled or not."
  ON)
message(STATUS "Building tests: ${BIM_BUILD_TESTS}")

option(BIM_ENABLE_TRACY "Enable Tracy profiler." OFF)
message(STATUS "Profiling with Tracy: ${BIM_ENABLE_TRACY}")

option(BIM_BUILD_SERVER
  "Whether the standalone server should be built or not."
  ON)

option(
  BIM_ANDROID_DEV
  "Create an Android build for developers, with a different app ID."
  OFF
)
message(STATUS "Developer build is ${BIM_ANDROID_DEV}")

option(
  BIM_PURE_FOSS
  "Use free software libraries exclusively."
  OFF
)
message(STATUS "Building with FOSS software only: ${BIM_PURE_FOSS}")


if(BIM_TARGET STREQUAL "android")
  set(BIM_ANDROID_GENERATED_RES_DIR
    "${CMAKE_BINARY_DIR}/android/app/res"
    CACHE
    PATH
    "Path to the directory where the generated Android resources will be \
produced."
  )
endif()

if(BIM_BUILDING_FOR_ANDROID)
  # Boost's GDB scripts use inline assembly that is not compatible
  # with armv7 targets.
  add_compile_definitions(BOOST_ALL_NO_EMBEDDED_GDB_SCRIPTS)
endif()
