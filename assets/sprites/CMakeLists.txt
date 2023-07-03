set(sprites
  "${CMAKE_CURRENT_SOURCE_DIR}/guy.png"
)

find_program(auto_pack_my_sprites auto-pack-my-sprites REQUIRED)

add_custom_command(
  OUTPUT
    ${generated_assets_dir}/sprites/sprite-sheet.png
    ${generated_assets_dir}/sprites/sprite-sheet.plist
  COMMAND ${auto_pack_my_sprites}
    --output sprite-sheet
    --no-crop
    --no-rotation
    --format plist
    --size 1024x1024
    --strip-paths
    ${sprites}
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${generated_assets_dir}/sprites
  COMMAND
    mv sprite-sheet.* ${generated_assets_dir}/sprites
  DEPENDS ${sprites}
)

add_custom_target(
  sprite-sheets ALL DEPENDS ${generated_assets_dir}/sprites/sprite-sheet.png
)
