// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/user_profiles.hpp>

#include <bim/app/business/user_profile.hpp>

#include <iscool/json/cast.hpp>
#include <iscool/json/cast_int64.hpp>

#include <json/value.h>

bool bim::app::from_json(user_profiles_response& u, const Json::Value& json)
{
  const Json::Value& profiles = json["profiles"];

  if (!profiles.isArray())
    return false;

  u.profiles.resize(profiles.size());

  for (Json::ArrayIndex i = 0, n = profiles.size(); i != n; ++i)
    {
      const Json::Value& profile = profiles[i];

      u.profiles[i].nickname =
          iscool::json::member_cast<std::string>(profile, "nickname");
      u.profiles[i].user_id =
          iscool::json::member_cast<bim::net::user_id>(profile, "user_id");
    }

  return true;
}
