// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/user_id.hpp>

#include <cstdint>
#include <string>

namespace bim::app
{
  struct user_profile
  {
    std::string nickname;
    bim::net::user_id user_id;
  };
}
