add_subdirectory(axmol)

if(BIM_BUILDING_FOR_ANDROID)
  add_subdirectory(android)
else()
  add_subdirectory(linux)
  add_subdirectory(player)
  add_subdirectory(server)
endif()
