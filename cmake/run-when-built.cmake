function(run_when_built target)
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND ${target}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endfunction()
