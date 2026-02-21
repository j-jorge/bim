set(main_sources
  main/src/bim/app/config.cpp
  main/src/bim/app/matchmaking_wait_message.cpp
  main/src/bim/app/player_progress_tracker.cpp

  main/src/bim/app/analytics/button_clicked.cpp
  main/src/bim/app/analytics/coins_transaction.cpp
  main/src/bim/app/analytics/error.cpp

  main/src/bim/app/preference/arena_stats.cpp
  main/src/bim/app/preference/audio.cpp
  main/src/bim/app/preference/controls.cpp
  main/src/bim/app/preference/date_of_next_version_update_message.cpp
  main/src/bim/app/preference/feature_flags.cpp
  main/src/bim/app/preference/haptic.cpp
  main/src/bim/app/preference/update_preferences.cpp
  main/src/bim/app/preference/user_language.cpp
  main/src/bim/app/preference/wallet.cpp
)

if (BIM_BUILDING_FOR_ANDROID)
  set(platform_sources
    android/src/bim/app/analytics_service.cpp
    android/src/bim/app/shop_service.cpp
    android/src/bim/app/shop_support.cpp
  )
  set(platform_includes android/include)
else()
  set(platform_sources
    desktop/src/bim/app/analytics_service.cpp
    desktop/src/bim/app/shop_service.cpp
    desktop/src/bim/app/shop_support.cpp
  )
  set(platform_includes desktop/include)
endif()

add_library(bim_app STATIC ${main_sources} ${platform_sources})
target_include_directories(bim_app PUBLIC main/include ${platform_includes})

target_link_libraries(bim_app
  PUBLIC
  bim_net
  iscool_i18n
  iscool_preferences
  iscool_system
)

if (NOT BIM_BUILD_TESTS)
  return()
endif()

add_executable(app-tests
  tests/src/bim/app/config.cpp
  tests/src/bim/app/preference/update_preferences.cpp
)
target_link_libraries(
  app-tests
  PRIVATE
  bim_app
  GTest::gtest
  bim_gtest_main
)
