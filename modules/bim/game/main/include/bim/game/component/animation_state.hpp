// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/animation/animation_id.hpp>

#include <chrono>

namespace bim::game
{
  struct animation_state
  {
    animation_id model;
    std::chrono::milliseconds elapsed_time;

    void transition_to(animation_id m);
  };
}
