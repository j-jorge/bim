find_program(style_merger style-merger REQUIRED)

set(common_style_files
  layout/center.json
)

set(main_task_style_files

  main-task.json

  action/rotate-forever-30s.json

  app/screen-wheel.json
  app/screen/bomb-button.json
  app/screen/directional-pad.json
  app/screen/end-game.json
  app/screen/end-game-bounds.json
  app/screen/joystick.json
  app/screen/lobby.json
  app/screen/lobby-bounds.json
  app/screen/matchmaking.json
  app/screen/matchmaking-bounds.json
  app/screen/online-game.json
  app/screen/online-game-bounds.json
  app/screen/shop.json
  app/screen/shop-bounds.json

  app/screen/font/stripe-message.json
  app/screen/font/stripe-title.json

  app/screen/lobby/stat-background.json
  app/screen/lobby/stat-background-bounds.json
  app/screen/lobby/stat-title.json
  app/screen/lobby/stat-value-label.json
  app/screen/lobby/stats-bounds.json
  app/screen/lobby/stats.json

  app/screen/online-game/control-panel-decorum.json
  app/screen/online-game/control-panel-edging.json
  app/screen/online-game/fog-display.json

  app/screen/part/feature-deck.json
  app/screen/part/feature-deck-bounds.json
  app/screen/part/feature-deck-button-bounds.json
  app/screen/part/matchmaking-feature-label.json
  app/screen/part/matchmaking-feature-label-bounds.json
  app/screen/part/matchmaking-player-label.json
  app/screen/part/menu-background.json
  app/screen/part/menu-background-bounds.json
  app/screen/part/menu-stripe.json
  app/screen/part/menu-stripe-bounds.json
  app/screen/part/player.json
  app/screen/part/wallet.json

  app/screen/shop/amount-label.json
  app/screen/shop/amount-label-bounds.json
  app/screen/shop/item.json
  app/screen/shop/item-bounds.json
  app/screen/shop/price-button.json
  app/screen/shop/price-button-bounds.json

  app/widget/game-feature-button.json

  button/back.json
  button/base.json
  button/feature-unavailable.json
  button/flat-with-label.json
  button/green-with-label.json
  button/red-with-label.json
  button/sprite.json
  button/thick-base.json
  button/yellow-with-icon.json
  button/yellow-with-label.json
  button/white-with-icon.json

  font/bold.json
  font/bold-center-normal-size.json
  font/bold-center-normal-size-with-outline.json
  font/center-normal-size.json
  font/regular.json
  font/fancy-regular.json
  font/fancy-center-with-outline-and-shadow.json

  layout/back-button.json
  layout/left-center.json
  layout/right-center.json
  layout/fill.json
  layout/fit.json

  nine-slice/price-tag.json
  nine-slice/rounded-dark-rectangle.json
  nine-slice/rounded-rectangle.json
  nine-slice/stripe.json

  popup/debug.json
  popup/full-screen.json
  popup/language.json
  popup/message.json
  popup/settings.json
  popup/small.json
  popup/yes_no_message.json

  popup/debug/font/main.json

  popup/language/button.json

  popup/message/base.json
  popup/message/message-bounds.json

  popup/settings/font/main.json
  popup/settings/font/small.json
  popup/settings/toggle/controls.json

  toggle/base.json
  toggle/flat.json
  toggle/flat-empty.json
  toggle/game-feature.json

  ${common_style_files}
)

set(main_scene_style_files
  launch/main-scene.json

  ${common_style_files}
)

add_custom_target(styles ALL)

function(merge_styles main_style_id)
  jsonlint("${main_style_id}.json" linted_file_files)

  foreach(file ${ARGN})
    jsonlint("${CMAKE_CURRENT_LIST_DIR}/${file}" linted_file)
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
