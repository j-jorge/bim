// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <string>

namespace bim::axmol::app
{
  struct script_info
  {
    std::string file_path;
    std::chrono::seconds timeout;
    bool number_screenshots;
    bool passed;
  };
}
