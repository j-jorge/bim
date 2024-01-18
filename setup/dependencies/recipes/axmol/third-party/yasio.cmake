set(yasio_include_dirs ${third_party_root}/)

install(
  DIRECTORY ${yasio_include_dirs}/yasio
  DESTINATION ${header_install_dir}
)
