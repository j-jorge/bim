find_package(Python3 REQUIRED COMPONENTS Interpreter)

function(jsonlint file target_var)
  get_filename_component(input "${file}" ABSOLUTE)
  cmake_path(
    RELATIVE_PATH input
    BASE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    OUTPUT_VARIABLE relative
  )
  set(target "${CMAKE_CURRENT_BINARY_DIR}/${relative}")
  get_filename_component(target_dir "${target}" DIRECTORY)

  set("${target_var}" "${relative}" PARENT_SCOPE)

  add_custom_command(
    OUTPUT "${relative}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${target_dir}"
    COMMAND "${Python3_EXECUTABLE}" -m json.tool "${input}" "${target}"
    DEPENDS "${input}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )
endfunction()
