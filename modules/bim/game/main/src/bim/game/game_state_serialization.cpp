// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/game_state_serialization.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/arena_reduction_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/crate.hpp>
#include <bim/game/component/crushed.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_blocker.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/fog_of_war.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/game_timer.hpp>
#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/invisibility_power_up_spawner.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/kicked.hpp>
#include <bim/game/component/layer_front.hpp>
#include <bim/game/component/layer_zero.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/power_up.hpp>
#include <bim/game/component/shallow.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/component/shield_power_up_spawner.hpp>
#include <bim/game/component/solid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/component/wall.hpp>

#include <bim/tracy.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/snapshot.hpp>

#include <algorithm>

namespace
{
  class input_archive
  {
  public:
    explicit input_archive(const char* storage)
      : m_cursor(storage)
    {}

    template <typename T>
    void operator()(T& value)
    {
      char* data = reinterpret_cast<char*>(std::addressof(value));
      std::copy(m_cursor, m_cursor + sizeof(T), data);
      m_cursor += sizeof(T);
    }

    template <typename T>
    void operator()(entt::entity& entity, T& value)
    {
      operator()(entity);
      operator()(value);
    }

  private:
    const char* m_cursor;
  };
}

namespace
{
  class output_archive
  {
  public:
    explicit output_archive(bim::game::archive_storage& storage)
      : m_storage(storage)
    {}

    template <typename T>
    void operator()(const T& value) const
    {
      const char* data = reinterpret_cast<const char*>(std::addressof(value));
      m_storage.insert(m_storage.end(), data, data + sizeof(T));
    }

    template <typename T>
    void operator()(entt::entity entity, const T& value) const
    {
      operator()(entity);
      operator()(value);
    }

  private:
    bim::game::archive_storage& m_storage;
  };
}

template <typename Archive, typename Snapshot>
static void archive_io(Snapshot&& snapshot, Archive&& archive)
{
  snapshot.template get<entt::entity>(archive)
      .template get<bim::game::animation_state>(archive)
      .template get<bim::game::arena_reduction_state>(archive)
      .template get<bim::game::bomb>(archive)
      .template get<bim::game::bomb_power_up>(archive)
      .template get<bim::game::bomb_power_up_spawner>(archive)
      .template get<bim::game::burning>(archive)
      .template get<bim::game::crate>(archive)
      .template get<bim::game::crushed>(archive)
      .template get<bim::game::dead>(archive)
      .template get<bim::game::falling_block>(archive)
      .template get<bim::game::flame>(archive)
      .template get<bim::game::flame_blocker>(archive)
      .template get<bim::game::flame_power_up>(archive)
      .template get<bim::game::flame_power_up_spawner>(archive)
      .template get<bim::game::fog_of_war>(archive)
      .template get<bim::game::fractional_position_on_grid>(archive)
      .template get<bim::game::game_timer>(archive)
      .template get<bim::game::invincibility_state>(archive)
      .template get<bim::game::invisibility_power_up>(archive)
      .template get<bim::game::invisibility_power_up_spawner>(archive)
      .template get<bim::game::invisibility_state>(archive)
      .template get<bim::game::kicked>(archive)
      .template get<bim::game::layer_front>(archive)
      .template get<bim::game::layer_zero>(archive)
      .template get<bim::game::player>(archive)
      .template get<bim::game::player_action>(archive)
      .template get<bim::game::player_action_queue>(archive)
      .template get<bim::game::position_on_grid>(archive)
      .template get<bim::game::power_up>(archive)
      .template get<bim::game::shallow>(archive)
      .template get<bim::game::shield>(archive)
      .template get<bim::game::shield_power_up>(archive)
      .template get<bim::game::shield_power_up_spawner>(archive)
      .template get<bim::game::solid>(archive)
      .template get<bim::game::timer>(archive)
      .template get<bim::game::wall>(archive);
}

void bim::game::serialize_state(archive_storage& storage,
                                const entt::registry& registry)
{
  ZoneScoped;

  storage.clear();

  archive_io(entt::snapshot(registry), output_archive(storage));
}

void bim::game::deserialize_state(entt::registry& registry,
                                  const archive_storage& storage)
{
  ZoneScoped;

  registry.clear();

  archive_io(entt::snapshot_loader(registry), input_archive(storage.data()));
}
