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
endif()

# The variable is not set in the recursive invocation for a specific
# Android architecture, which indicates that we do not need to
# configure the assets (which are common to all target architectures).
if (NOT BIM_TARGET)
  return()
endif()

include(jsonlint)
add_subdirectory(sprites)
add_subdirectory(style)

add_custom_target(assets ALL DEPENDS styles sprite-sheets)
