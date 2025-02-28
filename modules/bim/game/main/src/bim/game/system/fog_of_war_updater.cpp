// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/fog_of_war_updater.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/fog_of_war.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/constant/fog_roll_in_duration.hpp>

#include <bim/table_2d.impl.hpp>

#include <entt/entity/registry.hpp>

#include <cassert>

template class bim::table_2d<bim::game::fog_of_war*>;

static constexpr std::chrono::milliseconds g_fog_blow_duration(100);
static constexpr std::chrono::milliseconds g_fog_restore_duration(200);
static constexpr std::chrono::milliseconds g_fog_hide_duration(100);

namespace bim::game::detail
{
  static void add_neighbor(const fog_properties& p, int x, int y,
                           cell_neighborhood n)
  {
    if ((x < 0) || ((std::size_t)x >= p.fog.width()) || (y < 0)
        || ((std::size_t)y >= p.fog.height()))
      return;

    fog_of_war* const f = p.fog(x, y);

    if (!f)
      return;

    f->neighborhood |= n;
  }

  static void remove_neighbor(const fog_properties& p, int x, int y,
                              cell_neighborhood n)
  {
    if ((x < 0) || ((std::size_t)x >= p.fog.width()) || (y < 0)
        || ((std::size_t)y >= p.fog.height()))
      return;

    fog_of_war* const f = p.fog(x, y);

    if (!f)
      return;

    f->neighborhood &= ~n;
  }

  static void uncover(const fog_properties& p, int x, int y)
  {
    remove_neighbor(p, x - 1, y - 1, cell_neighborhood::down_right);
    remove_neighbor(p, x, y - 1, cell_neighborhood::down);
    remove_neighbor(p, x + 1, y - 1, cell_neighborhood::down_left);

    remove_neighbor(p, x - 1, y, cell_neighborhood::right);
    remove_neighbor(p, x + 1, y, cell_neighborhood::left);

    remove_neighbor(p, x - 1, y + 1, cell_neighborhood::up_right);
    remove_neighbor(p, x, y + 1, cell_neighborhood::up);
    remove_neighbor(p, x + 1, y + 1, cell_neighborhood::up_left);
  }

  static void hide(const fog_properties& p, int x, int y)
  {
    if ((x < 0) || ((std::size_t)x >= p.fog.width()) || (y < 0)
        || ((std::size_t)y >= p.fog.height()))
      return;

    fog_of_war* const f = p.fog(x, y);

    if (!f || (f->state == fog_state::hiding))
      return;

    f->state = fog_state::hiding;

    assert(p.timer(x, y) != nullptr);
    p.timer(x, y)->duration =
        f->opacity * g_fog_hide_duration / fog_of_war::full_opacity;

    uncover(p, x, y);
  }

  static void blow(const fog_properties& p, int x, int y)
  {
    fog_of_war* const f = p.fog(x, y);

    if (!f || (f->state == fog_state::hiding)
        || (f->state == fog_state::blown))
      return;

    f->state = fog_state::blown;

    timer* const t = p.timer(x, y);
    assert(t != nullptr);
    t->duration = f->opacity * g_fog_blow_duration / fog_of_war::full_opacity;

    uncover(p, x, y);
  }

  static void uncover_around_player(const fog_properties& p, int player_x,
                                    int player_y)
  {
    hide(p, player_x - 1, player_y - 1);
    hide(p, player_x, player_y - 1);
    hide(p, player_x + 1, player_y - 1);

    hide(p, player_x - 1, player_y);
    hide(p, player_x, player_y);
    hide(p, player_x + 1, player_y);

    hide(p, player_x - 1, player_y + 1);
    hide(p, player_x, player_y + 1);
    hide(p, player_x + 1, player_y + 1);
  }
}

bim::game::fog_of_war_updater::fog_of_war_updater(entt::registry& registry,
                                                  const arena& arena,
                                                  std::uint8_t player_count)
  : m_blown(arena.width(), arena.height())
  , m_player_count(player_count)
{
  for (int i = 0; i != player_count; ++i)
    {
      m_tables[i].fog =
          bim::table_2d<fog_of_war*>(arena.width(), arena.height(), nullptr);
      m_tables[i].timer =
          bim::table_2d<timer*>(arena.width(), arena.height(), nullptr);
    }
}

bim::game::fog_of_war_updater::~fog_of_war_updater() = default;

const bim::table_2d<bim::game::fog_of_war*>&
bim::game::fog_of_war_updater::fog(std::size_t player_index) const
{
  assert(player_index <= m_tables.size());
  return m_tables[player_index].fog;
}

void bim::game::fog_of_war_updater::update(entt::registry& registry)
{
  build_maps(registry);

  for (const auto& [_, player, position] :
       registry.view<player, fractional_position_on_grid>().each())
    update_fog(registry, player.index, position.grid_aligned_x(),
               position.grid_aligned_y());
}

void bim::game::fog_of_war_updater::update_fog(entt::registry& registry,
                                               std::size_t player_index,
                                               int player_x, int player_y)
{
  m_blown.fill(false);

  const detail::fog_properties& p = m_tables[player_index];
  detail::uncover_around_player(p, player_x, player_y);

  if (m_player_count <= 2)
    uncover_around_flames(registry, p);

  update_opacity_from_timers(registry, p);
}

void bim::game::fog_of_war_updater::build_maps(entt::registry& registry)
{
  for (detail::fog_properties& p : m_tables)
    {
      p.fog.fill(nullptr);
      p.timer.fill(nullptr);
    }

  registry.view<fog_of_war, timer, position_on_grid>().each(
      [&](fog_of_war& fog, timer& t, const position_on_grid& position) -> void
      {
        m_tables[fog.player_index].fog(position.x, position.y) = &fog;
        m_tables[fog.player_index].timer(position.x, position.y) = &t;
      });
}

void bim::game::fog_of_war_updater::uncover_around_flames(
    entt::registry& registry, const detail::fog_properties& p)
{
  registry.view<flame, position_on_grid>().each(
      [&](const flame&, const position_on_grid& position) -> void
      {
        m_blown(position.x, position.y) = true;
        detail::blow(p, position.x, position.y);
      });
}

void bim::game::fog_of_war_updater::update_opacity_from_timers(
    entt::registry& registry, const detail::fog_properties& p) const
{
  const std::size_t w = p.fog.width();
  const std::size_t h = p.fog.height();

  for (std::size_t y = 0; y != h; ++y)
    for (std::size_t x = 0; x != w; ++x)
      {
        fog_of_war* const f = p.fog(x, y);

        if (!f)
          continue;

        assert(p.timer(x, y) != nullptr);
        timer* const t = p.timer(x, y);

        switch (f->state)
          {
          case fog_state::stable:
            break;
          case fog_state::blown:
            {
              const std::chrono::milliseconds num = t->duration;
              const std::chrono::milliseconds denum = g_fog_blow_duration;
              f->opacity =
                  num.count() * fog_of_war::full_opacity / denum.count();

              // Do not restore the flame if it has been blown in the current
              // iteration.
              if (!m_blown(x, y) && (t->duration.count() == 0))
                {
                  f->state = fog_state::restore;
                  t->duration = g_fog_restore_duration;
                }
            }
            break;
          case fog_state::restore:
          case fog_state::roll_in:
            {
              const std::chrono::milliseconds denum =
                  (f->state == fog_state::roll_in) ? g_fog_roll_in_duration
                                                   : g_fog_restore_duration;

              if (t->duration.count() == 0)
                {
                  f->state = fog_state::stable;

                  detail::add_neighbor(p, x - 1, y - 1,
                                       cell_neighborhood::down_right);
                  detail::add_neighbor(p, x, y - 1, cell_neighborhood::down);
                  detail::add_neighbor(p, x + 1, y - 1,
                                       cell_neighborhood::down_left);

                  detail::add_neighbor(p, x - 1, y, cell_neighborhood::right);
                  detail::add_neighbor(p, x + 1, y, cell_neighborhood::left);

                  detail::add_neighbor(p, x - 1, y + 1,
                                       cell_neighborhood::up_right);
                  detail::add_neighbor(p, x, y + 1, cell_neighborhood::up);
                  detail::add_neighbor(p, x + 1, y + 1,
                                       cell_neighborhood::up_left);
                }

              const std::chrono::milliseconds num = denum - t->duration;
              f->opacity = std::max<int>(0, num.count())
                           * fog_of_war::full_opacity / denum.count();
            }
            break;
          case fog_state::hiding:
            {
              const std::chrono::milliseconds num = t->duration;
              const std::chrono::milliseconds denum = g_fog_hide_duration;
              f->opacity =
                  num.count() * fog_of_war::full_opacity / denum.count();
            }
            break;
          }
      }
}
