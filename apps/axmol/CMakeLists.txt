find_package(axmol 1.0.0 REQUIRED)

add_library(bim_app
  STATIC
  src/bim/axmol/app/alloc_assets.cpp
  src/bim/axmol/app/application.cpp
  src/bim/axmol/app/arena_display_config.cpp
  src/bim/axmol/app/bridge.cpp
  src/bim/axmol/app/config.cpp
  src/bim/axmol/app/feature_deck.cpp
  src/bim/axmol/app/fog_display.cpp
  src/bim/axmol/app/main_scene.cpp
  src/bim/axmol/app/matchmaking_wait_message.cpp
  src/bim/axmol/app/player_progress_tracker.cpp
  src/bim/axmol/app/root_scene.cpp
  src/bim/axmol/app/scene_lock.cpp
  src/bim/axmol/app/screen_wheel.cpp

  src/bim/axmol/app/part/wallet.cpp

  src/bim/axmol/app/popup/debug_popup.cpp
  src/bim/axmol/app/popup/message.cpp
  src/bim/axmol/app/popup/popup.cpp
  src/bim/axmol/app/popup/settings_popup.cpp

  src/bim/axmol/app/preference/arena_stats.cpp
  src/bim/axmol/app/preference/audio.cpp
  src/bim/axmol/app/preference/controls.cpp
  src/bim/axmol/app/preference/date_of_next_config_update.cpp
  src/bim/axmol/app/preference/date_of_next_version_update_message.cpp
  src/bim/axmol/app/preference/feature_flags.cpp
  src/bim/axmol/app/preference/haptic.cpp
  src/bim/axmol/app/preference/update_preferences.cpp
  src/bim/axmol/app/preference/wallet.cpp

  src/bim/axmol/app/screen/end_game.cpp
  src/bim/axmol/app/screen/lobby.cpp
  src/bim/axmol/app/screen/matchmaking.cpp
  src/bim/axmol/app/screen/online_game.cpp

  src/bim/axmol/app/task/main_task.cpp

  src/bim/axmol/app/widget/game_feature_button.cpp
  src/bim/axmol/app/widget/player.cpp
)
target_include_directories(bim_app PUBLIC include)
target_link_libraries(bim_app
  PUBLIC
  axmol::axmol
  bim_axmol_audio
  bim_axmol_display
  bim_axmol_files
  bim_axmol_http
  bim_axmol_input
  bim_axmol_schedule
  bim_axmol_widget
  bim_net
  iscool_i18n
  iscool_preferences
  iscool_signals
  iscool_social
  iscool_system
  iscool_style
)
