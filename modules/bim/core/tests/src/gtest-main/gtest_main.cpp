// SPDX-License-Identifier: AGPL-3.0-only
#include <iscool/log/enable_console_log.hpp>

#include <string_view>

#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
  bool ignore_failures = false;
  bool show_help = false;

  for (int argi = 1; argi < argc; ++argi)
    if (argv[argi] == std::string_view("--console-log"))
      iscool::log::enable_console_log();
    else if (argv[argi] == std::string_view("--ignore-failures"))
      ignore_failures = true;
    else if (argv[argi] == std::string_view("--help"))
      show_help = true;

  testing::InitGoogleTest(&argc, argv);

  const int result = RUN_ALL_TESTS();

  if (show_help)
    std::cout << R"(
Custom Options:
  --console-log
      Display logs in the terminal.
  --ignore-failures
      Close the program with exit code 0 even if there are failing tests. The
      goal of this option is to distinguish errors from the tests from errors
      detected by instrumentation tools (e.g. Valgrind).
)";

  if (ignore_failures)
    return 0;

  return result;
}
