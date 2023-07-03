set(generated_assets_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

foreach(shader ${axmol_shader_files})
  get_filename_component(basename ${shader} NAME)
  configure_file(${shader} ${generated_assets_dir}/${basename} COPYONLY)
endforeach()

add_subdirectory(sprites)
