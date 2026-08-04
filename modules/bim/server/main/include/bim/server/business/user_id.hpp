// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/user_id.hpp>

#include <vector>

namespace Json
{
  class Value;
}

namespace bim::server::business
{
  struct user_id_response
  {
    std::vector<bim::net::client_token> rejected;
    std::vector<bim::net::client_token> accepted;
    std::vector<bim::net::user_id> user_id;
  };

  bool from_json(user_id_response& r, const Json::Value& json);
}
