// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/table_2d.hpp>

#include <entt/entity/fwd.hpp>

#include <boost/container/small_vector.hpp>

#include <span>

namespace bim::game
{
  class entity_world_map
  {
  public:
    entity_world_map();
    entity_world_map(std::uint8_t width, std::uint8_t height);
    entity_world_map(const entity_world_map& that);
    entity_world_map(entity_world_map&& that);
    ~entity_world_map();

    entity_world_map& operator=(const entity_world_map& that);
    entity_world_map& operator=(entity_world_map&& that);

    std::span<const entt::entity> entities_at(std::uint8_t x,
                                              std::uint8_t y) const;
    void put_entity(entt::entity e, std::uint8_t x, std::uint8_t y);
    void erase_entity(entt::entity e, std::uint8_t x, std::uint8_t y);
    void erase_entities(std::uint8_t x, std::uint8_t y);

  private:
    using entity_vector = boost::container::small_vector<entt::entity, 8>;

  private:
    table_2d<entity_vector> m_entities;
  };
}
