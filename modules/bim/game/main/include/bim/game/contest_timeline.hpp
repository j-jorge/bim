// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest_fingerprint.hpp>

#include <entt/entity/fwd.hpp>

#include <array>
#include <cstdio>
#include <vector>

namespace bim::game
{
  class contest_timeline;
  class player_action;

  bool load_contest_timeline(contest_timeline& timeline, std::FILE* f);

  class contest_timeline
  {
    friend bool load_contest_timeline(contest_timeline& timeline,
                                      std::FILE* file);

  public:
    contest_timeline();
    ~contest_timeline();

    const bim::game::contest_fingerprint& fingerprint() const;
    std::size_t tick_count() const;

    void load_tick(std::uint32_t tick, entt::registry& registry) const;

  private:
    bim::game::contest_fingerprint m_fingerprint;
    std::vector<bim::game::player_action> m_actions;

    std::array<std::uint32_t, g_max_player_count> m_kick_event_tick;
    std::array<std::uint8_t, g_max_player_count> m_kick_event_player;
    std::uint8_t m_kick_event_count;
  };
}
