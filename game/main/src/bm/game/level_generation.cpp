#include <bm/game/level_generation.hpp>

#include <bm/game/assume.hpp>
#include <bm/game/wall.hpp>

#include <entt/entity/registry.hpp>

void bm::game::generate_basic_level(entt::registry& registry, arena& arena)
{
  const int width = arena.width();
  const int height = arena.height();

  bm_assume(width >= 3);
  bm_assume(height >= 3);

  // Unbreakable walls on the borders.
  for(int x = 0; x != width; ++x)
    registry.emplace<wall>(arena.at(x, 0));

  for(int x = 0; x != width; ++x)
    registry.emplace<wall>(arena.at(x, height - 1));

  for(int y = 1; y < height - 1; ++y)
    registry.emplace<wall>(arena.at(0, y));

  for(int y = 1; y < height - 1; ++y)
    registry.emplace<wall>(arena.at(width - 1, y));

  // Unbreakable walls in the game area.
  for(int y = 2; y < height - 1; y += 2)
    for(int x = 2; x < width - 1; x += 2)
      registry.emplace<wall>(arena.at(x, y));
}
