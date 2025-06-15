find_package(axmol 1.0.0 REQUIRED)

set(main_sources
  main/src/bim/axmol/app/alloc_assets.cpp
  main/src/bim/axmol/app/application.cpp
  main/src/bim/axmol/app/arena_display_config.cpp
  main/src/bim/axmol/app/bridge.cpp
  main/src/bim/axmol/app/config.cpp
  main/src/bim/axmol/app/feature_deck.cpp
  main/src/bim/axmol/app/fog_display.cpp
  main/src/bim/axmol/app/main_scene.cpp
  main/src/bim/axmol/app/matchmaking_wait_message.cpp
  main/src/bim/axmol/app/player_progress_tracker.cpp
  main/src/bim/axmol/app/root_scene.cpp
  main/src/bim/axmol/app/scene_lock.cpp
  main/src/bim/axmol/app/screen_wheel.cpp

  main/src/bim/axmol/app/part/wallet.cpp

  main/src/bim/axmol/app/popup/debug_popup.cpp
  main/src/bim/axmol/app/popup/message.cpp
  main/src/bim/axmol/app/popup/popup.cpp
  main/src/bim/axmol/app/popup/settings_popup.cpp

  main/src/bim/axmol/app/preference/arena_stats.cpp
  main/src/bim/axmol/app/preference/audio.cpp
  main/src/bim/axmol/app/preference/controls.cpp
  main/src/bim/axmol/app/preference/date_of_next_config_update.cpp
  main/src/bim/axmol/app/preference/date_of_next_version_update_message.cpp
  main/src/bim/axmol/app/preference/feature_flags.cpp
  main/src/bim/axmol/app/preference/haptic.cpp
  main/src/bim/axmol/app/preference/update_preferences.cpp
  main/src/bim/axmol/app/preference/wallet.cpp

  main/src/bim/axmol/app/screen/end_game.cpp
  main/src/bim/axmol/app/screen/lobby.cpp
  main/src/bim/axmol/app/screen/matchmaking.cpp
  main/src/bim/axmol/app/screen/online_game.cpp
  main/src/bim/axmol/app/screen/shop.cpp

  main/src/bim/axmol/app/task/main_task.cpp

  main/src/bim/axmol/app/widget/game_feature_button.cpp
  main/src/bim/axmol/app/widget/player.cpp
)

if (BIM_BUILDING_FOR_ANDROID)
  set(platform_sources
    android/src/bim/axmol/app/shop_service.cpp
    android/src/bim/axmol/app/shop_support.cpp
  )
  set(platform_includes android/include)
else()
  set(platform_sources
    desktop/src/bim/axmol/app/shop_service.cpp
    desktop/src/bim/axmol/app/shop_support.cpp
  )
  set(platform_includes desktop/include)
endif()

add_library(bim_app STATIC ${main_sources} ${platform_sources})
target_include_directories(bim_app PUBLIC main/include ${platform_includes})

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
