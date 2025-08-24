find_package(Gettext REQUIRED)

set(languages de en fr pt pt_BR)

# When building the Android app we use the .po files as they are. On
# the other hand, for Linux builds we generate the .pot file and
# update the .po files, which will then be added into the repository
# by the developer.
if(NOT BIM_TARGET STREQUAL "linux")
  foreach(language ${languages})
    set(po_files ${po_files} ${CMAKE_CURRENT_LIST_DIR}/${language}.po)
  endforeach()
else()
  function(collect_target_sources target output_variable)
    get_property(target_sources TARGET ${target} PROPERTY SOURCES)
    get_property(target_dir TARGET ${target} PROPERTY SOURCE_DIR)

    foreach(source ${target_sources})
      cmake_path(IS_ABSOLUTE source is_absolute)

      if(NOT is_absolute)
        set(source ${target_dir}/${source})
      endif()

      set(sources ${sources} ${source})
    endforeach()

    set(${output_variable} ${sources} PARENT_SCOPE)
  endfunction()

  collect_target_sources(bim_app bim_app_sources)
  collect_target_sources(styles style_files)

  # Collect the strings from the styles.
  set(style_strings_source "${CMAKE_CURRENT_BINARY_DIR}/style-strings.c")
  add_custom_command(
    OUTPUT ${style_strings_source}
    COMMAND
      ${CMAKE_CURRENT_LIST_DIR}/strings-from-style.py
      ${style_strings_source}
      ${style_files}
    DEPENDS
      ${CMAKE_CURRENT_LIST_DIR}/strings-from-style.py
      ${style_files}
  )

  # Extract the strings from the sources.
  find_program(xgettext xgettext REQUIRED)
  set(pot_file ${CMAKE_CURRENT_BINARY_DIR}/bim.pot)
  add_custom_command(
    OUTPUT ${pot_file}
    COMMAND
      ${xgettext}
      --keyword=ic_gettext
      --keyword=ic_ngettext:1,2
      --from-code=utf-8
      --no-location
      --output=${pot_file}
      ${bim_app_sources}
      ${style_strings_source}
    COMMAND
    # The .pot file may be copied in the repository by a custom
    # command below. In order to avoid committing changes where only
    # the creation date is modified, we simply remove the creation
    # date from the .pot file.
      ${CMAKE_COMMAND}
      -DPOT_FILE=${pot_file}
      -P ${CMAKE_CURRENT_LIST_DIR}/erase-pot-creation-date.cmake
    DEPENDS
      ${bim_app_sources} ${style_strings_source} erase-pot-creation-date.cmake
  )

  # Merge the .po files from the repository with .pot file created
  # from the the messages found in the game.
  foreach(language ${languages})
    set(input ${CMAKE_CURRENT_LIST_DIR}/${language}.po)
    set(po_files ${po_files} ${CMAKE_CURRENT_BINARY_DIR}/${language}.po)
    add_custom_command(
      OUTPUT ${language}.po
      COMMAND
        ${CMAKE_COMMAND}
        -E copy
        ${input}
        ${language}.po
      COMMAND
        ${GETTEXT_MSGMERGE_EXECUTABLE}
        --update
        ${language}.po
        ${pot_file}
      DEPENDS ${pot_file} ${input}
    )
  endforeach()

  option(
    BIM_UPDATE_TRANSLATIONS
    "Update the .po files from the repository when ON."
    OFF
  )

  if(BIM_UPDATE_TRANSLATIONS)
    set(update_translations_all ALL)
  endif()

  add_custom_target(update-translations
    ${update_translations_all}
    COMMAND
      ${CMAKE_COMMAND}
      -E copy_if_different
      ${po_files}
      ${pot_file}
      ${CMAKE_CURRENT_LIST_DIR}/
    DEPENDS ${po_files} ${pot_file}
  )
endif()

set(generated_i18n_dir ${BIM_GENERATED_ASSETS_DIR}/i18n)

foreach(po_file ${po_files})
  cmake_path(GET po_file STEM basename)
  set(mo_file ${generated_i18n_dir}/${basename}.mo)
  set(mo_files ${mo_files} ${mo_file})

  add_custom_command(
    OUTPUT ${mo_file}
    COMMAND
      ${CMAKE_COMMAND} -E make_directory ${generated_i18n_dir}
    COMMAND
      ${GETTEXT_MSGFMT_EXECUTABLE} --output-file=${mo_file} ${po_file}
    DEPENDS ${po_file}
  )
endforeach()

add_custom_target(translations DEPENDS ${mo_files})
