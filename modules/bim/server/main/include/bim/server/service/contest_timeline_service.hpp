// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/contest_timeline_writer.hpp>

#include <iscool/net/message/channel_id.hpp>

#include <filesystem>

namespace bim::game
{
  struct contest_fingerprint;
}

namespace bim::server
{
  class contest_timeline_service
  {
  public:
    explicit contest_timeline_service(std::filesystem::path dir);

    bim::game::contest_timeline_writer
    open(iscool::net::channel_id channel,
         const bim::game::contest_fingerprint& contest);

  private:
    const std::filesystem::path m_directory;
  };
}
