set(main_sources
  main/src/bim/axmol/app/alloc_assets.cpp
  main/src/bim/axmol/app/application.cpp
  main/src/bim/axmol/app/arena_display_config.cpp
  main/src/bim/axmol/app/bridge.cpp
  main/src/bim/axmol/app/fog_display.cpp
  main/src/bim/axmol/app/frame_profiler.cpp
  main/src/bim/axmol/app/game_feature_sprite_frame.cpp
  main/src/bim/axmol/app/loading_screen.cpp
  main/src/bim/axmol/app/main_scene.cpp
  main/src/bim/axmol/app/root_scene.cpp
  main/src/bim/axmol/app/screen_wheel.cpp

  main/src/bim/axmol/app/part/wallet.cpp

  main/src/bim/axmol/app/popup/debug_popup.cpp
  main/src/bim/axmol/app/popup/language_popup.cpp
  main/src/bim/axmol/app/popup/message.cpp
  main/src/bim/axmol/app/popup/player_statistics_popup.cpp
  main/src/bim/axmol/app/popup/popup.cpp
  main/src/bim/axmol/app/popup/settings_popup.cpp

  main/src/bim/axmol/app/screen/end_game.cpp
  main/src/bim/axmol/app/screen/game_features.cpp
  main/src/bim/axmol/app/screen/lobby.cpp
  main/src/bim/axmol/app/screen/matchmaking.cpp
  main/src/bim/axmol/app/screen/online_game.cpp
  main/src/bim/axmol/app/screen/shop.cpp

  main/src/bim/axmol/app/task/main_task.cpp

  main/src/bim/axmol/app/widget/game_feature_button.cpp
  main/src/bim/axmol/app/widget/feature_deck.cpp
  main/src/bim/axmol/app/widget/player.cpp
  main/src/bim/axmol/app/widget/register_widgets.cpp
)

add_library(bim_axmol_app STATIC ${main_sources})
target_include_directories(bim_axmol_app PUBLIC main/include)

target_link_libraries(bim_axmol_app
  PUBLIC
  axmol::axmol
  bim_axmol_audio
  bim_axmol_display
  bim_axmol_files
  bim_axmol_http
  bim_axmol_input
  bim_axmol_schedule
  bim_axmol_widget
  bim_app
  bim_net
  iscool_signals
  iscool_social
  iscool_system
  iscool_style
)
