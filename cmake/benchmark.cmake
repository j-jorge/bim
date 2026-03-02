if (BIM_BUILD_BENCHMARKS)
  find_package(benchmark REQUIRED)
  add_custom_target(benchmarks)

  function(add_benchmark target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "LINK;SOURCES")

    add_executable(
      ${target}
      ${arg_SOURCES}
    )
    target_link_libraries(
      ${target}
      PRIVATE
      ${arg_LINK}
      benchmark::benchmark
      benchmark::benchmark_main
    )
    add_dependencies(benchmarks ${target})
  endfunction()
else()
  function(add_benchmark target)
  endfunction()
endif()
