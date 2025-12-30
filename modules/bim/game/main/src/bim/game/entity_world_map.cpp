// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

#include <bim/table_2d.impl.hpp>

#include <cassert>

bim::game::entity_world_map::entity_world_map() = default;

bim::game::entity_world_map::entity_world_map(std::uint8_t width,
                                              std::uint8_t height)
  : m_entities(width, height)
{}

bim::game::entity_world_map::entity_world_map(const entity_world_map& that) =
    default;
bim::game::entity_world_map::entity_world_map(entity_world_map&& that) =
    default;
bim::game::entity_world_map::~entity_world_map() = default;

bim::game::entity_world_map&
bim::game::entity_world_map::operator=(const entity_world_map& that) = default;

bim::game::entity_world_map&
bim::game::entity_world_map::operator=(entity_world_map&& that) = default;

std::span<const entt::entity>
bim::game::entity_world_map::entities_at(std::uint8_t x, std::uint8_t y) const
{
  return m_entities(x, y);
}

void bim::game::entity_world_map::put_entity(entt::entity e, std::uint8_t x,
                                             std::uint8_t y)
{
#ifndef NDEBUG
  for (const entity_vector& v : m_entities)
    assert(std::ranges::find(v, e) == v.end());
#endif

  m_entities(x, y).push_back(e);
}

void bim::game::entity_world_map::erase_entity(entt::entity e, std::uint8_t x,
                                               std::uint8_t y)
{
  entity_vector& v = m_entities(x, y);

  v.erase(std::remove(v.begin(), v.end(), e), v.end());
}

void bim::game::entity_world_map::erase_entities(std::uint8_t x,
                                                 std::uint8_t y)
{
  m_entities(x, y).clear();
}
