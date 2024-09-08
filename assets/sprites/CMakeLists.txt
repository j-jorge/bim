set(sprites
  "sprites/bomb.png"
  "sprites/brick-wall.png"
  "sprites/player-1.png"
  "sprites/player-2.png"
  "sprites/player-3.png"
  "sprites/player-4.png"
  "sprites/power-up-bomb.png"
  "sprites/power-up-flame.png"
  "sprites/solid-wall.png"

  "sprites/menu/glow.png"
  "sprites/menu/rays.png"
  "sprites/menu/rounded-button-border-54-66.png"
  "sprites/menu/rounded-button-fill-54-66.png"
  "sprites/menu/silhouette-1.png"
  "sprites/menu/silhouette-2.png"
  "sprites/menu/silhouette-3.png"
  "sprites/menu/silhouette-4.png"
  "sprites/menu/stripe-1-79-1-258.png"

  "sprites/popup/popup-border-146-146.png"
  "sprites/popup/popup-fill-146-146.png"

  "sprites/ui/directional-pad-left.png"
  "sprites/ui/directional-pad-right.png"
  "sprites/ui/directional-pad-left-on.png"
  "sprites/ui/directional-pad-right-on.png"
  "sprites/ui/directional-pad-up.png"
  "sprites/ui/directional-pad-up-on.png"
  "sprites/ui/directional-pad-down-on.png"
  "sprites/ui/directional-pad-down.png"
)
set(sprites_bleeding
  "sprites/flame-arm.png"
  "sprites/flame-center.png"
  "sprites/flame-end.png"

  "sprites/menu/horizontal-gradient.png"
  "sprites/menu/vertical-gradient.png"
  "sprites/menu/vertical-gray-gradient.png"
  "sprites/menu/vertical-green-gradient.png"
  "sprites/menu/vertical-red-gradient.png"
  "sprites/menu/vertical-yellow-gradient.png"
)

foreach(s ${sprites} ${sprites_bleeding})
  set(sprites_full_path ${sprites_full_path} ${CMAKE_CURRENT_LIST_DIR}/../${s})
endforeach()

find_program(auto_pack_my_sprites auto-pack-my-sprites REQUIRED)

# Handling the paths for the sprite sheet is quite complex. We want
# relative paths in the plist, so we pass the PNGs via relative paths,
# and consequently we must change the working directory. Moreover the
# --output argument does not work well with paths thus we generate the
# resulting files in the current working directory then we move them
# to the build directory. Note that the current working directory is
# in the source tree, so if we ever intentionally add a
# sprite-sheet.png file there, we are going to have problems.
add_custom_command(
  OUTPUT
    ${BIM_GENERATED_ASSETS_DIR}/sprite-sheet.png
    ${BIM_GENERATED_ASSETS_DIR}/sprite-sheet.plist
  COMMAND ${auto_pack_my_sprites}
    --output sprite-sheet
    --no-crop
    --no-rotation
    --format plist
    --size 1024x1024
    ${sprites}
    --bleeding
    ${sprites_bleeding}
  COMMAND
    mv sprite-sheet.* ${BIM_GENERATED_ASSETS_DIR}
  DEPENDS ${sprites_full_path}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
)

add_custom_target(
  sprite-sheets ALL DEPENDS ${BIM_GENERATED_ASSETS_DIR}/sprite-sheet.png
)
