// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/archive_storage.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  void serialize_state(archive_storage& storage,
                       const entt::registry& registry);
  void deserialize_state(entt::registry& registry,
                         const archive_storage& storage);
}
