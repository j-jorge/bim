// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/business/user_id.hpp>

#include <iscool/json/cast_int64.hpp>

#include <json/value.h>

#include <cassert>

bool bim::server::business::from_json(user_id_response& r,
                                      const Json::Value& json)
{
  if (!json.isObject())
    return false;

  r.rejected.clear();
  r.accepted.clear();
  r.user_id.clear();

  const Json::Value& tokens = json["tokens"];
  const Json::Value& users = json["user_ids"];

  if (!tokens.isArray() || !users.isArray() || (tokens.size() != users.size()))
    return false;

  const Json::ArrayIndex n = tokens.size();

  r.accepted.reserve(n);
  r.user_id.reserve(n);

  for (Json::ArrayIndex i = 0; i != n; ++i)
    {
      const bim::net::client_token token(
          iscool::json::cast<bim::net::client_token>(tokens[i]));

      const Json::Value& user = users[i];

      if (user == Json::nullValue)
        {
          r.rejected.emplace_back(token);
          continue;
        }

      const bim::net::user_id user_id(
          iscool::json::cast<bim::net::user_id>(users[i]));
      assert(user_id != 0);

      r.accepted.emplace_back(token);
      r.user_id.emplace_back(user_id);
    }

  return true;
}
