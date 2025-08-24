foreach(shader shine.frag)
  set(source_shaders ${source_shaders} ${CMAKE_CURRENT_LIST_DIR}/${shader})
  get_filename_component(generated_shader ${shader} NAME_WE)
  set(generated_shader ${generated_shader}_fs)

  set(ax_shader ${CMAKE_BINARY_DIR}/runtime/axslc/${generated_shader})
  set(bim_shader_dir ${BIM_GENERATED_ASSETS_DIR}/shaders)
  set(bim_shader ${bim_shader_dir}/${generated_shader})

  set(generated_shader_files ${generated_shader_files} ${bim_shader})

  add_custom_command(
    OUTPUT ${bim_shader}
    COMMAND
      ${CMAKE_COMMAND} -E make_directory ${bim_shader_dir}
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different ${ax_shader} ${bim_shader_dir}
    DEPENDS ${ax_shader}
  )
endforeach()

add_custom_target(shaders_internal ALL)
ax_target_compile_shaders(
  shaders_internal
  FILES
  ${source_shaders}
)

add_custom_target(shaders DEPENDS ${generated_shader_files})

