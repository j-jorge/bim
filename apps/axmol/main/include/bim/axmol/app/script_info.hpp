// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <string>

namespace bim::axmol::app
{
  struct script_info
  {
    std::string file_path;
    bool number_screenshots;
    bool passed;
  };
}
