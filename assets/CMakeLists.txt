# BIM_INSTALL_AXMOL_SHADERS is passed for one Android target such that
# we can acess the shaders from the installation of Axmol. They would
# be incorrect if we took them from the host.
if (BIM_INSTALL_AXMOL_SHADERS OR BIM_TARGET STREQUAL "linux")
  if (NOT axmol_shader_files)
    message(
      WARNING "No shader file for axmol? It is suspicious. Check its package."
    )
  endif()

  foreach(shader ${axmol_shader_files})
    get_filename_component(basename ${shader} NAME)
    configure_file(${shader} ${BIM_GENERATED_ASSETS_DIR}/${basename} COPYONLY)
  endforeach()

  add_subdirectory(shaders)
endif()

# The variable is not set in the recursive invocation for a specific
# Android architecture, which indicates that we do not need to
# configure the assets (which are common to all target architectures).
if (NOT BIM_TARGET)
  return()
endif()

include(jsonlint)

foreach(file animations.json colors.json resources.json)
  json_minify(
    "${CMAKE_CURRENT_LIST_DIR}/${file}"
    "${BIM_GENERATED_ASSETS_DIR}/${file}"
    minified_json_files
  )
endforeach()

add_custom_target(minified_json DEPENDS ${minified_json_files})

add_subdirectory(sprites)
add_subdirectory(style)
add_subdirectory(texture)

# The styles must be processed before i18n.
add_subdirectory(i18n)

add_custom_target(
  assets
  ALL
  DEPENDS
  ${minified_json_files}
  shaders
  styles
  sprite-sheets
  textures
  translations
)
