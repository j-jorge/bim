find_program(style_merger style-merger REQUIRED)

set(common_style_files
  ${CMAKE_CURRENT_LIST_DIR}/layout/center.json
)

set(main_task_style_files
  ${CMAKE_CURRENT_LIST_DIR}/main-task.json

  ${CMAKE_CURRENT_LIST_DIR}/app/screen-wheel.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/bomb-button.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/directional-pad.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/end-game.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/end-game-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/joystick.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/lobby.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/lobby-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/matchmaking.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/matchmaking-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/online-game.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/online-game-bounds.json

  ${CMAKE_CURRENT_LIST_DIR}/app/screen/font/stripe-message.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/font/stripe-title.json

  ${CMAKE_CURRENT_LIST_DIR}/app/screen/online-game/control-panel-decorum.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/online-game/control-panel-edging.json

  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/feature-deck.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/feature-deck-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/feature-deck-button-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/matchmaking-feature-label.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/matchmaking-feature-label-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/matchmaking-player-label.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/menu-background.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/menu-background-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/menu-stripe.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/menu-stripe-bounds.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/player-1.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/player-2.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/player-3.json
  ${CMAKE_CURRENT_LIST_DIR}/app/screen/part/player-4.json

  ${CMAKE_CURRENT_LIST_DIR}/button/base.json
  ${CMAKE_CURRENT_LIST_DIR}/button/feature-unavailable.json
  ${CMAKE_CURRENT_LIST_DIR}/button/flat-with-label.json
  ${CMAKE_CURRENT_LIST_DIR}/button/green-with-label.json
  ${CMAKE_CURRENT_LIST_DIR}/button/red-with-label.json
  ${CMAKE_CURRENT_LIST_DIR}/button/sprite.json
  ${CMAKE_CURRENT_LIST_DIR}/button/thick-base.json
  ${CMAKE_CURRENT_LIST_DIR}/button/yellow-with-icon.json
  ${CMAKE_CURRENT_LIST_DIR}/button/yellow-with-label.json

  ${CMAKE_CURRENT_LIST_DIR}/font/bold.json
  ${CMAKE_CURRENT_LIST_DIR}/font/bold-center-normal-size.json
  ${CMAKE_CURRENT_LIST_DIR}/font/bold-center-normal-size-with-outline.json
  ${CMAKE_CURRENT_LIST_DIR}/font/center-normal-size.json
  ${CMAKE_CURRENT_LIST_DIR}/font/regular.json
  ${CMAKE_CURRENT_LIST_DIR}/font/fancy-regular.json

  ${CMAKE_CURRENT_LIST_DIR}/layout/left-center.json
  ${CMAKE_CURRENT_LIST_DIR}/layout/right-center.json
  ${CMAKE_CURRENT_LIST_DIR}/layout/fill.json
  ${CMAKE_CURRENT_LIST_DIR}/layout/fit.json

  ${CMAKE_CURRENT_LIST_DIR}/nine-slice/rounded-rectangle.json
  ${CMAKE_CURRENT_LIST_DIR}/nine-slice/stripe.json

  ${CMAKE_CURRENT_LIST_DIR}/popup/debug.json
  ${CMAKE_CURRENT_LIST_DIR}/popup/full-screen.json
  ${CMAKE_CURRENT_LIST_DIR}/popup/message.json
  ${CMAKE_CURRENT_LIST_DIR}/popup/settings.json
  ${CMAKE_CURRENT_LIST_DIR}/popup/small.json

  ${CMAKE_CURRENT_LIST_DIR}/popup/settings/font/main.json
  ${CMAKE_CURRENT_LIST_DIR}/popup/settings/font/small.json
  ${CMAKE_CURRENT_LIST_DIR}/popup/settings/toggle/controls.json

  ${CMAKE_CURRENT_LIST_DIR}/popup/debug/font/main.json

  ${CMAKE_CURRENT_LIST_DIR}/toggle/base.json
  ${CMAKE_CURRENT_LIST_DIR}/toggle/feature.json
  ${CMAKE_CURRENT_LIST_DIR}/toggle/flat.json
  ${CMAKE_CURRENT_LIST_DIR}/toggle/flat-empty.json

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

merge_styles(main-task ${main_task_style_files})
merge_styles(launch/main-scene ${main_scene_style_files})
