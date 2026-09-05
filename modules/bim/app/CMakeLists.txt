configure_file(business_url.hpp.in include/bim/app/business_url.hpp @ONLY)

set(main_sources
  main/src/bim/app/config.cpp
  main/src/bim/app/device_id.cpp
  main/src/bim/app/matchmaking_wait_message.cpp
  main/src/bim/app/shop_service.cpp

  main/src/bim/app/analytics/button_clicked.cpp
  main/src/bim/app/analytics/coins_transaction.cpp
  main/src/bim/app/analytics/error.cpp

  main/src/bim/app/business/consume_reward.cpp
  main/src/bim/app/business/legacy_state.cpp
  main/src/bim/app/business/player_profile.cpp
  main/src/bim/app/business/player_profile_json.cpp
  main/src/bim/app/business/validate_purchase.cpp

  main/src/bim/app/job/assign_feature_slots_job.cpp
  main/src/bim/app/job/authenticate_with_business_job.cpp
  main/src/bim/app/job/buy_feature_job.cpp
  main/src/bim/app/job/buy_feature_slot_job.cpp
  main/src/bim/app/job/consume_game_reward_job.cpp
  main/src/bim/app/job/fetch_config_job.cpp
  main/src/bim/app/job/fetch_player_profile_job.cpp
  main/src/bim/app/job/push_legacy_state_job.cpp
  main/src/bim/app/job/validate_purchase_job.cpp

  main/src/bim/app/preference/audio.cpp
  main/src/bim/app/preference/controls.cpp
  main/src/bim/app/preference/date_of_next_version_update_message.cpp
  main/src/bim/app/preference/haptic.cpp
  main/src/bim/app/preference/legacy.cpp
  main/src/bim/app/preference/update_preferences.cpp
  main/src/bim/app/preference/user_language.cpp
)

if (BIM_BUILDING_FOR_ANDROID)
  set(platform_sources
    android/src/bim/app/analytics_service.cpp
    android/src/bim/app/shop_implementation.cpp
    android/src/bim/app/shop_support.cpp
  )
  set(platform_includes android/include)
else()
  set(platform_sources
    desktop/src/bim/app/analytics_service.cpp
    desktop/src/bim/app/shop_implementation.cpp
    desktop/src/bim/app/shop_support.cpp
  )
  set(platform_includes desktop/include)
endif()

add_library(bim_app STATIC ${main_sources} ${platform_sources})
target_include_directories(
  bim_app
  PUBLIC
  main/include
  ${platform_includes}
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(bim_app
  PUBLIC
  bim_business
  bim_net
  iscool_http
  iscool_i18n
  iscool_preferences
  iscool_system
)

if (NOT BIM_BUILD_TESTS)
  return()
endif()

add_executable(app-tests
  tests/src/bim/app/config.cpp
  tests/src/bim/app/business/player_profile_json.cpp

  tests/src/bim/app/job/assign_feature_slots_job.cpp
  tests/src/bim/app/job/buy_feature_job.cpp
  tests/src/bim/app/job/buy_feature_slot_job.cpp
  tests/src/bim/app/job/consume_game_reward_job.cpp
  tests/src/bim/app/job/validate_purchase_job.cpp

  tests/src/bim/app/tests/http_service.cpp
  tests/src/bim/app/tests/scheduler.cpp
)
target_include_directories(
  app-tests
  PRIVATE
  tests/include
)
target_link_libraries(
  app-tests
  PRIVATE
  bim_app
  GTest::gtest
  bim_gtest_main
)
