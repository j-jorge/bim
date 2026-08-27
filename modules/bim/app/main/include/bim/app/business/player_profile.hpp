// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/app/constant/game_feature_slot_count.hpp>

#include <bim/net/message/user_id.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <cstdint>
#include <string>

namespace bim::app
{
  class player_profile
  {
  public:
    bim::game::feature_flags enabled_feature_flags() const;

  public:
    std::string nickname;
    std::int64_t coins;

    bim::net::user_id user_id;

    std::array<bool, bim::app::g_game_feature_slot_count> slot_availability;
    std::array<bim::game::feature_flags, bim::app::g_game_feature_slot_count>
        slot_feature;

    bim::game::feature_flags available_features;

    std::int64_t arena_victories;
    std::int64_t arena_defeats;
    std::int64_t arena_draws;
  };
}
