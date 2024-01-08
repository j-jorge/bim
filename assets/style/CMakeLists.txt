find_program(style_merger style-merger REQUIRED)

set(all_style_files
  ${CMAKE_CURRENT_LIST_DIR}/button/flat-with-label.json
  ${CMAKE_CURRENT_LIST_DIR}/font/regular.json
  ${CMAKE_CURRENT_LIST_DIR}/launch/main-scene.json
  ${CMAKE_CURRENT_LIST_DIR}/layout/fill.json
)

foreach(file ${all_style_files})
  jsonlint("${file}" linted_file)
  list(APPEND linted_style_files "${linted_file}")
endforeach()

add_custom_command(
  OUTPUT
    ${generated_assets_dir}/style/launch/main-scene.json
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${generated_assets_dir}/style/launch
  COMMAND
    ${style_merger}
    --root ${CMAKE_CURRENT_LIST_DIR}/../
    --search-path style/
    launch/main-scene
    > ${generated_assets_dir}/style/launch/main-scene.json
  DEPENDS ${linted_style_files}
)

add_custom_target(
  styles ALL DEPENDS ${generated_assets_dir}/style/launch/main-scene.json
)
