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
