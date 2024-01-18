add_subdirectory(axmol)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  add_subdirectory(android)
else()
  add_subdirectory(linux)
  add_subdirectory(console)
  add_subdirectory(server)
endif()
