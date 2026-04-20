// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/contest_fingerprint.hpp>
#include <bim/game/per_player_array.hpp>

#include <entt/entity/fwd.hpp>

#include <array>
#include <cstdio>
#include <vector>

namespace bim::game
{
  class contest_timeline;
  struct player_action;

  bool load_contest_timeline(contest_timeline& timeline, std::FILE* f);

  class contest_timeline
  {
    friend bool load_contest_timeline(contest_timeline& timeline,
                                      std::FILE* file);

  public:
    contest_timeline();
    ~contest_timeline();

    int game_version() const;
    const bim::game::contest_fingerprint& fingerprint() const;
    const per_player_array<bool>& bot() const;

    std::size_t tick_count() const;

    void load_tick(std::uint32_t tick, entt::registry& registry) const;

  private:
    int m_game_version;
    bim::game::contest_fingerprint m_fingerprint;
    per_player_array<bool> m_bot;

    std::vector<bim::game::player_action> m_actions;

    per_player_array<std::uint32_t> m_kick_event_tick;
    per_player_array<std::uint8_t> m_kick_event_player;
    std::uint8_t m_kick_event_count;
  };
}
