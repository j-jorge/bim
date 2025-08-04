add_custom_target(shaders_internal ALL)
ax_target_compile_shaders(
  shaders_internal
  FILES
  ${CMAKE_CURRENT_LIST_DIR}/shine.frag
)

add_custom_target(shaders
   COMMAND mkdir -p "${BIM_GENERATED_ASSETS_DIR}/shaders"
   COMMAND
     ${CMAKE_COMMAND}
     -E copy_directory ${CMAKE_BINARY_DIR}/runtime/axslc/
     "${BIM_GENERATED_ASSETS_DIR}/shaders"
   DEPENDS shaders_internal
 )
