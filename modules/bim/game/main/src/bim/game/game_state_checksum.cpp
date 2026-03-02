// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/game_state_checksum.hpp>

#include <bim/game/all_components.hpp>

#include <bim/crc32.hpp>
#include <bim/tracy.hpp>

#include <iscool/meta/underlying_type.hpp>
#include <iscool/net/endianness.hpp>

#include <entt/entity/registry.hpp>

#include <type_traits>

namespace
{
  struct checksum_state
  {
    std::array<char, 4096> bytes;
    std::size_t length;
    std::uint32_t result;
  };
}

static void push_checksum_bytes(checksum_state& checksum, const char* value,
                                std::size_t n)
{
  if (checksum.length + n > checksum.bytes.size())
    {
      checksum.result = bim::crc32(
          std::span(&checksum.bytes[0], checksum.length), checksum.result);
      checksum.length = 0;
    }

  std::copy_n(value, n, &checksum.bytes[checksum.length]);
  checksum.length += n;
}

template <typename T>
  requires requires { sizeof(T) == 1; }
static void push_checksum_bytes(checksum_state& checksum, T value)
{
  push_checksum_bytes(checksum, (const char*)&value, 1);
}

template <typename T>
static void push_checksum_bytes(checksum_state& checksum, T value)
{
  using raw_type = iscool::meta::underlying_type<T>::type;
  using unsigned_type =
      std::conditional_t<std::is_signed_v<raw_type>,
                         std::make_unsigned_t<raw_type>, raw_type>;

  const unsigned_type v =
      iscool::net::to_network_endianness((unsigned_type)value);

  const char* data = reinterpret_cast<const char*>(std::addressof(v));
  constexpr std::size_t n = sizeof(raw_type);
  push_checksum_bytes(checksum, data, n);
}

static void update_checksum(checksum_state& checksum_state, entt::entity e)
{
  push_checksum_bytes(checksum_state, entt::to_entity(e));
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::animation_state& c)
{
  push_checksum_bytes(checksum_state, c.model);
  push_checksum_bytes(checksum_state, c.elapsed_time.count());
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::arena_reduction_state& c)
{
  push_checksum_bytes(checksum_state, c.index_of_next_fall);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::bomb& c)
{
  push_checksum_bytes(checksum_state, c.strength);
  push_checksum_bytes(checksum_state, c.player_index);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::flame& c)
{
  push_checksum_bytes(checksum_state, c.direction);
  push_checksum_bytes(checksum_state, c.segment);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::fog_of_war& c)
{
  push_checksum_bytes(checksum_state, c.player_index);
  push_checksum_bytes(checksum_state, c.opacity);
  push_checksum_bytes(checksum_state, c.neighborhood);
  push_checksum_bytes(checksum_state, c.state);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::fractional_position_on_grid& c)
{
  push_checksum_bytes(checksum_state, c.x.raw_value());
  push_checksum_bytes(checksum_state, c.y.raw_value());
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::invincibility_state& c)
{
  update_checksum(checksum_state, c.entity);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::invisibility_state& c)
{
  update_checksum(checksum_state, c.entity);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::player_action& c)
{
  push_checksum_bytes(checksum_state, c.movement);
  push_checksum_bytes(checksum_state, c.drop_bomb);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::player_action_queue& c)
{
  for (const bim::game::queued_action& a : c.m_queue)
    {
      update_checksum(checksum_state, a.action);
      push_checksum_bytes(checksum_state, a.arena_x);
      push_checksum_bytes(checksum_state, a.arena_y);
    }
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::player& c)
{
  push_checksum_bytes(checksum_state, c.index);
  push_checksum_bytes(checksum_state, c.bomb_capacity);
  push_checksum_bytes(checksum_state, c.bomb_available);
  push_checksum_bytes(checksum_state, c.bomb_strength);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::position_on_grid& c)
{
  push_checksum_bytes(checksum_state, c.x);
  push_checksum_bytes(checksum_state, c.y);
}

static void update_checksum(checksum_state& checksum_state,
                            const bim::game::timer& c)
{
  push_checksum_bytes(checksum_state, c.duration.count());
}

template <typename T>
static void
update_checksum_for_component(checksum_state& checksum_state,
                              const entt::registry& registry,
                              const std::vector<entt::entity>& all_entities)
{
  std::uint16_t n = 0;

  if constexpr (std::is_empty_v<T>)
    for (entt::entity e : all_entities)
      {
        const entt::registry::storage_for_type<T>* const storage =
            registry.storage<T>();

        if (!storage || !storage->contains(e))
          continue;

        update_checksum(checksum_state, e);
        ++n;
      }
  else
    for (entt::entity e : all_entities)
      {
        const T* const c = registry.try_get<T>(e);

        if (!c)
          continue;

        update_checksum(checksum_state, e);
        update_checksum(checksum_state, *c);
        ++n;
      }

  push_checksum_bytes(checksum_state, n);
}

std::uint32_t bim::game::game_state_checksum(const entt::registry& registry)
{
  ZoneScoped;

  std::vector<entt::entity> all_entities;
  all_entities.reserve(1024);

  for (const entt::entity e : registry.view<entt::entity>())
    all_entities.push_back(e);

  assert(all_entities.capacity() <= 1024);

  std::sort(all_entities.begin(), all_entities.end(),
            [](entt::entity lhs, entt::entity rhs)
              {
                return entt::to_entity(lhs) < entt::to_entity(rhs);
              });

  checksum_state checksum;
  checksum.length = 0;
  checksum.result = 0;

  for (std::size_t i = 0, n = all_entities.size(); i != n; ++i)
    update_checksum(checksum, all_entities[i]);

  push_checksum_bytes(checksum, all_entities.size());

  std::uint8_t type_index = 0;

#define bim_game_x_component(n)                                               \
  update_checksum_for_component<n>(checksum, registry, all_entities);         \
  push_checksum_bytes(checksum, type_index);                                  \
  ++type_index;
#include <bim/game/for_each_component.hpp>

  checksum.result = bim::crc32(std::span(&checksum.bytes[0], checksum.length),
                               checksum.result);

  return checksum.result;
}
