set(main_sources
  main/src/bim/app/config.cpp
  main/src/bim/app/matchmaking_wait_message.cpp
  main/src/bim/app/player_progress_tracker.cpp

  main/src/bim/app/preference/arena_stats.cpp
  main/src/bim/app/preference/audio.cpp
  main/src/bim/app/preference/controls.cpp
  main/src/bim/app/preference/date_of_next_config_update.cpp
  main/src/bim/app/preference/date_of_next_version_update_message.cpp
  main/src/bim/app/preference/feature_flags.cpp
  main/src/bim/app/preference/haptic.cpp
  main/src/bim/app/preference/update_preferences.cpp
  main/src/bim/app/preference/user_language.cpp
  main/src/bim/app/preference/wallet.cpp
)

add_library(bim_app STATIC ${main_sources})
target_include_directories(bim_app PUBLIC main/include)

target_link_libraries(bim_app
  PUBLIC
  bim_net
  iscool_i18n
  iscool_preferences
  iscool_system
)
