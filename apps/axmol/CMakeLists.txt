find_package(axmol 1.0.0 REQUIRED)

add_library(bim_app
  STATIC
  src/bim/axmol/app/application.cpp
  src/bim/axmol/app/bridge.cpp
  src/bim/axmol/app/main_scene.cpp
  src/bim/axmol/app/root_scene.cpp
  src/bim/axmol/app/scene_lock.cpp
  src/bim/axmol/app/screen_wheel.cpp

  src/bim/axmol/app/popup/message.cpp
  src/bim/axmol/app/popup/popup.cpp
  src/bim/axmol/app/popup/settings_popup.cpp

  src/bim/axmol/app/preference/audio.cpp
  src/bim/axmol/app/preference/haptic.cpp
  src/bim/axmol/app/preference/network.cpp

  src/bim/axmol/app/screen/end_game.cpp
  src/bim/axmol/app/screen/lobby.cpp
  src/bim/axmol/app/screen/matchmaking.cpp
  src/bim/axmol/app/screen/online_game.cpp

  src/bim/axmol/app/task/main_task.cpp
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
  iscool_system
  iscool_style
  iscool_wish
)
