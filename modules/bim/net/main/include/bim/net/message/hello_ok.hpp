// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/message_type.hpp>
#include <bim/net/message/version.hpp>

#include <iscool/net/message/raw_message.hpp>

#include <string>

namespace bim::net
{
  class hello_ok
  {
  public:
    static iscool::net::message_type get_type()
    {
      return message_type::hello_ok;
    }

    hello_ok();
    explicit hello_ok(const iscool::net::byte_array& raw_content);

    void build_message(iscool::net::message& message) const;

  public:
    client_token request_token;
    bim::net::version version;

    std::uint32_t games_now;
    std::uint32_t games_last_hour;
    std::uint32_t games_last_day;
    std::uint32_t games_last_month;

    std::uint32_t sessions_now;
    std::uint32_t sessions_last_hour;
    std::uint32_t sessions_last_day;
    std::uint32_t sessions_last_month;

    std::string name;
  };
}
