// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/contest_timeline_writer.hpp>

#include <iscool/net/message/channel_id.hpp>

#include <string>

namespace bim::game
{
  struct contest_fingerprint;
}

namespace bim::server
{
  struct config;

  class contest_timeline_service
  {
  public:
    explicit contest_timeline_service(const config& config);

    bim::game::contest_timeline_writer
    open(iscool::net::channel_id channel,
         const bim::game::contest_fingerprint& contest);

  private:
    const std::string m_directory;
  };
}
