// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/arena.hpp>

#include <bim/game/static_wall.hpp>

#include <bim/assume.hpp>
#include <bim/table_2d.hpp>

#include <entt/entity/entity.hpp>

#include <vector>


bim::game::arena::arena() = default;

bim::game::arena::arena(std::uint8_t width, std::uint8_t height)
  : m_width(width)
  , m_height(height)
  , m_entities(width, height, (entt::entity)entt::null)
  , m_is_static_wall(width, height, false)
  , m_static_walls(width, height, std::nullopt)
  , m_solids(width, height, false)
{
}

bim::game::arena::arena(const arena& that) noexcept = default;
bim::game::arena::arena(arena&& that) noexcept = default;
bim::game::arena::~arena() = default;
bim::game::arena&
bim::game::arena::operator=(const arena& that) noexcept = default;
bim::game::arena& bim::game::arena::operator=(arena&& that) noexcept = default;

std::uint8_t bim::game::arena::width() const
{
  return m_width;
}

std::uint8_t bim::game::arena::height() const
{
  return m_height;
}

entt::entity bim::game::arena::entity_at(std::uint8_t x, std::uint8_t y) const
{
  return m_entities(x,y);
}

void bim::game::arena::put_entity(std::uint8_t x, std::uint8_t y,
                                  entt::entity e)
{
  assert(entity_at(x, y) == entt::null);
  m_entities(x,y) = e;
}

void bim::game::arena::erase_entity(std::uint8_t x, std::uint8_t y)
{
  m_entities(x,y) = entt::null;
  m_solids(x,y) = false;
}

bool bim::game::arena::is_solid(std::uint8_t x, std::uint8_t y) const
{
  return m_solids(x,y);
}

void bim::game::arena::set_solid(std::uint8_t x, std::uint8_t y)
{
  m_solids(x, y) = true;
}

std::vector<bim::game::static_wall> bim::game::arena::static_walls() const {
  std::vector<static_wall> result;
  for (const auto& opt : m_static_walls) {
      if (opt.has_value()) {
          result.push_back(*opt);
      }
  }
  return result;
}


bool bim::game::arena::is_static_wall(std::uint8_t x, std::uint8_t y) const
{
  return m_is_static_wall(x,y);

}

void bim::game::arena::set_static_wall(std::uint8_t x, std::uint8_t y,
                                       cell_neighborhood n)
{
  m_static_walls(x,y) = static_wall(x,y,n);
  m_is_static_wall(x,y) = true;
  m_solids(x,y) = true;
}
