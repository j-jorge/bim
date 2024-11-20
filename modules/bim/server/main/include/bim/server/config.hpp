// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <string>

namespace bim::server
{
  struct config
  {
    unsigned short port;

    std::chrono::seconds game_service_clean_up_interval =
        std::chrono::minutes(3);

    std::string contest_timeline_folder;
    bool enable_contest_timeline_recording = false;
  };
}
