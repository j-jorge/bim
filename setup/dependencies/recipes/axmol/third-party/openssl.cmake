install(
  DIRECTORY ${third_party_root}/openssl/_x/lib/${third_party_system_dir}/
  DESTINATION ${library_install_dir}
)
if(target_linux)
  install(
    DIRECTORY
    ${third_party_root}/openssl/_x/include/${third_party_system_dir}/openssl
    DESTINATION ${header_install_dir}
  )
  install(
    DIRECTORY
    ${third_party_root}/openssl/_x/include/openssl
    DESTINATION ${header_install_dir}
  )
endif()
