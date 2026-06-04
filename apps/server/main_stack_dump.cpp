// SPDX-License-Identifier: AGPL-3.0-only
#include <cpptrace/cpptrace.hpp>

#include <cstdio>
#include <iostream>

int main()
{
  cpptrace::object_trace trace;
  cpptrace::safe_object_frame frame;

  while (true)
    {
      const std::size_t res = std::fread(&frame, sizeof(frame), 1, stdin);

      if (res == 0)
        break;

      if (res != 1)
        {
          std::fprintf(stderr, "Failed to read stack frame from stdin.\n");
          break;
        }

      trace.frames.push_back(frame.resolve());
    }

  trace.resolve().print(std::cerr);

  return 0;
}
