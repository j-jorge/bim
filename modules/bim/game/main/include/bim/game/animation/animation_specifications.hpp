// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/animation/animation_id.hpp>

#include <entt/entity/fwd.hpp>

#include <chrono>
#include <functional>

namespace bim::game
{
  struct animation_specifications
  {
    std::chrono::milliseconds duration{};
    animation_id next{};
    std::function<void(entt::registry&, entt::entity)> dispatch_completion;
  };
}
