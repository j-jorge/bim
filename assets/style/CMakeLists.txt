find_program(style_merger style-merger REQUIRED)

set(common_style_files
  ${CMAKE_CURRENT_LIST_DIR}/layout/fill.json
)

set(screen_wheel_style_files
  ${CMAKE_CURRENT_LIST_DIR}/app/screen-wheel.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/directional-pad.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/end-game.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/end-game-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/lobby.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/lobby-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/matchmaking.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/matchmaking-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/online-game.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/online-game-bounds.json

  ${CMAKE_CURRENT_LIST_DIR}/button/flat-with-label.json
  ${CMAKE_CURRENT_LIST_DIR}/button/sprite.json
  ${CMAKE_CURRENT_LIST_DIR}/button/yellow-with-label.json
  ${CMAKE_CURRENT_LIST_DIR}/font/bold.json
  ${CMAKE_CURRENT_LIST_DIR}/font/bold-center-normal-size-with-outline.json
  ${CMAKE_CURRENT_LIST_DIR}/font/regular.json
  ${CMAKE_CURRENT_LIST_DIR}/font/fancy-regular.json

  ${common_style_files}
)

set(main_scene_style_files
  ${CMAKE_CURRENT_LIST_DIR}/launch/main-scene.json

  ${common_style_files}
)

add_custom_target(styles ALL)

function(merge_styles main_style_id)
  jsonlint("${main_style_id}.json" linted_file_files)

  foreach(file ${ARGN})
    jsonlint("${file}" linted_file)
    list(APPEND linted_style_files "${linted_file}")
  endforeach()

  set(output "${BIM_GENERATED_ASSETS_DIR}/style/${main_style_id}.json")
  get_filename_component(output_dir "${output}" DIRECTORY)

  add_custom_command(
    OUTPUT "${output}"
    COMMAND
      ${CMAKE_COMMAND} -E make_directory "${output_dir}"
    COMMAND
      ${style_merger}
      --root ${CMAKE_CURRENT_LIST_DIR}/../
      --search-path style/
      ${main_style_id}
      > "${output}"
    DEPENDS ${linted_style_files}
  )

  set_property(TARGET styles APPEND PROPERTY SOURCES "${output}")
endfunction()

merge_styles(app/screen-wheel ${screen_wheel_style_files})
merge_styles(launch/main-scene ${main_scene_style_files})
