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
add_subdirectory(texture)

# The styles must be processed before i18n.
add_subdirectory(i18n)

add_custom_target(
  assets ALL DEPENDS styles sprite-sheets textures translations
)

if(NOT BIM_BUILDING_FOR_ANDROID)
  return()
endif()

find_program(convert convert REQUIRED)
find_program(identify identify REQUIRED)

set(master_icon "${CMAKE_CURRENT_LIST_DIR}/icon.png")
execute_process(
  COMMAND "${identify}" -ping -format "%w" "${master_icon}"
  OUTPUT_VARIABLE master_icon_size
  COMMAND_ERROR_IS_FATAL ANY
)

function(generate_legacy_icon output dpi size)
  set(output_dir "${BIM_ANDROID_GENERATED_RES_DIR}/mipmap-${dpi}/")
  set(output_file "${output_dir}/ic_launcher.png")
  set(${output} ${${output}} ${output_file} PARENT_SCOPE)

  # The master icon has extra margins that we are going to remove.
  math(EXPR ref_size "74 * ${master_icon_size} / 100")
  math(EXPR ref_offset "(${master_icon_size} -${ref_size}) / 2")
  # Size for the rounded corners.
  math(EXPR ref_size_20_percents "20 * ${ref_size} / 100")

  add_custom_command(
    OUTPUT "${output_file}"
    COMMAND
      "${CMAKE_COMMAND}" -E make_directory "${output_dir}"
    COMMAND
      "${convert}"
      "${master_icon}"
      # Remove the extra margin.
      -crop "${ref_size}x${ref_size}+${ref_offset}+${ref_offset}"
      # Rounded rectangle mask.
      -matte
      "\\\(" -size ${ref_size}x${ref_size}
        xc:none
        -draw
        "roundrectangle \
0,0,${ref_size},${ref_size},${ref_size_20_percents},${ref_size_20_percents}"
      "\\\)"
      -compose DstIn
      -composite
      -resize "${size}"
      "${output_file}"
    DEPENDS "${master_icon}"
    )
endfunction()

function(generate_adaptive_icon output dpi size)
  set(output_dir "${BIM_ANDROID_GENERATED_RES_DIR}/mipmap-${dpi}/")
  set(output_file "${output_dir}/ic_launcher_round.png")
  set(${output} ${${output}} ${output_file} PARENT_SCOPE)

  # The master icon has extra margins that we are going to remove.
  math(EXPR ref_size "125 * ${master_icon_size} / 100")

  add_custom_command(
    OUTPUT "${output_file}"
    COMMAND
      "${CMAKE_COMMAND}" -E make_directory "${output_dir}"
    COMMAND
      "${convert}"
      "${master_icon}"
      # Pick the color of the top-left pixel.
      -set background "%[pixel:p{0,0}]"
      -gravity center
      -extent ${ref_size}x${ref_size}
      "${output_file}"
    DEPENDS "${master_icon}"
    )
endfunction()

set(dpis xxxhdpi xxhdpi xhdpi hdpi mdpi dpi)
set(sizes 432 324 216 162 108 81)

foreach(dpi size IN ZIP_LISTS dpis sizes)
  generate_legacy_icon(icons ${dpi} ${size})
  generate_adaptive_icon(icons ${dpi} ${size})
endforeach()

add_custom_target(generated-icons ALL DEPENDS ${icons})
add_dependencies(assets generated-icons)
