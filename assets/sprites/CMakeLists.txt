set(sprites
  "sprites/bomb.png"
  "sprites/brick-wall.png"
  "sprites/guy.png"
  "sprites/player-1.png"
  "sprites/player-2.png"
  "sprites/player-3.png"
  "sprites/player-4.png"
  "sprites/solid-wall.png"
)

foreach(s ${sprites})
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
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${BIM_GENERATED_ASSETS_DIR}/sprites
  COMMAND ${auto_pack_my_sprites}
    --output sprite-sheet
    --no-crop
    --no-rotation
    --format plist
    --size 1024x1024
    ${sprites}
  COMMAND
    mv sprite-sheet.* ${BIM_GENERATED_ASSETS_DIR}
  DEPENDS ${sprites_full_path}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
)

add_custom_target(
  sprite-sheets ALL DEPENDS ${BIM_GENERATED_ASSETS_DIR}/sprite-sheet.png
)
