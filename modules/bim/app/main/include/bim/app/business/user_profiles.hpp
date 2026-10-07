// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <vector>

namespace Json
{
  class Value;
}

namespace bim::app
{
  struct user_profile;

  struct user_profiles_response
  {
    std::vector<user_profile> profiles;
  };

  bool from_json(user_profiles_response& r, const Json::Value& json);
}
