// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/contest_fingerprint.hpp>

#include <cstdio>
#include <span>
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
    const bim::game::contest_fingerprint& fingerprint() const;
    std::span<const bim::game::player_action> tick(std::size_t i) const;
    std::size_t tick_count() const;

  private:
    bim::game::contest_fingerprint m_fingerprint;
    std::vector<bim::game::player_action> m_actions;
  };
}
