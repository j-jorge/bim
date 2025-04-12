// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  class context;

  void animator(const context& context, entt::registry& registry,
                std::chrono::milliseconds elapsed_time);
}
