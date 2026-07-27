// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>

#include <iscool/net/message/session_id.hpp>

#include <cstdint>

namespace bim::server
{
  enum class create_session_result_state : std::uint8_t
  {
    rejected,
    accepted,
    pending
  };

  struct create_session_result
  {
    create_session_result_state state;
    bim::net::client_token token;
    iscool::net::session_id session;
  };
}
