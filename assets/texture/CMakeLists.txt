set(textures
  "bomb-stencil.png"
  "defeat-background-pattern.png"
  "draw-background-pattern.png"
  "gifts-background-pattern.png"
  "menu-background-pattern.png"
  "popup-background-pattern.png"
  "search-background-pattern.png"
  "squares.png"
  "stars.png"
  "victory-background-pattern.png"
)

find_program(magick magick REQUIRED)

set(generated_textures)
set(output_directory "${BIM_GENERATED_ASSETS_DIR}/texture")

foreach(t ${textures})
  set(output_file "${output_directory}/${t}")
  set(generated_textures ${generated_textures} "${output_file}")

  add_custom_command(
    OUTPUT "${output_file}"
    COMMAND
      ${CMAKE_COMMAND} -E make_directory "${output_directory}"
    COMMAND ${magick}
      ${t}
      -write mpr:image
      -background black
      -alpha Remove mpr:image
      -compose Copy_Opacity
      -composite
      -strip
      "${output_file}"
    DEPENDS ${t}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )
endforeach()

add_custom_target(
  textures ALL DEPENDS ${generated_textures}
)
