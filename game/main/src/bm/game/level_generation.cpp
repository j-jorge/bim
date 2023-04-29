#include <bm/game/level_generation.hpp>

#include <bm/game/assume.hpp>
#include <bm/game/brick_wall.hpp>
#include <bm/game/position_on_grid.hpp>
#include <bm/game/random_generator.hpp>

#include <entt/entity/registry.hpp>

#include <boost/random/uniform_int_distribution.hpp>

void bm::game::generate_basic_level_structure(arena& arena)
{
  const int width = arena.width();
  const int height = arena.height();

  bm_assume(width >= 3);
  bm_assume(height >= 3);

  // Unbreakable walls on the borders.
  for(int x = 0; x != width; ++x)
    arena.set_static_wall(x, 0);

  for(int x = 0; x != width; ++x)
    arena.set_static_wall(x, height - 1);

  for(int y = 1; y < height - 1; ++y)
    arena.set_static_wall(0, y);

  for(int y = 1; y < height - 1; ++y)
    arena.set_static_wall(width - 1, y);

  // Unbreakable walls in the game area.
  for(int y = 2; y < height - 1; y += 2)
    for(int x = 2; x < width - 1; x += 2)
      arena.set_static_wall(x, y);
}

void bm::game::insert_random_brick_walls(arena& arena,
                                         entt::registry& registry,
                                         random_generator& random_generator,
                                         std::uint8_t brick_wall_probability)
{
  const int width = arena.width();
  const int height = arena.height();

  boost::random::uniform_int_distribution<std::uint8_t> random(0, 99);

  constexpr auto new_brick_wall = [](entt::registry& registry, std::uint8_t x,
                                     std::uint8_t y) -> entt::entity
  {
    const entt::entity entity = registry.create();
    registry.emplace<bm::game::brick_wall>(entity);
    registry.emplace<bm::game::position_on_grid>(entity, x, y);

    return entity;
  };

  for(int y = 0; y != height; ++y)
    for(int x = 0; x != width; ++x)
      if(!arena.is_static_wall(x, y)
         && (random(random_generator) < brick_wall_probability))
        arena.put_entity(x, y, new_brick_wall(registry, x, y));
}
