// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/fog_display.hpp>

#include <bim/axmol/app/alloc_assets.hpp>

#include <bim/axmol/widget/factory/node.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/hide_while_visible.hpp>

#include <bim/game/component/fog_of_war.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/constant/default_arena_size.hpp>
#include <bim/game/contest.hpp>

#define x_widget_scope bim::axmol::app::fog_display::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::Node, fog_container)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <bim/table_2d.hpp>
#include <bim/tracy.hpp>

#include <axmol/2d/Sprite.h>
#include <axmol/2d/SpriteFrameCache.h>

#include <entt/entity/registry.hpp>

#include <cassert>

static bim::game::cell_neighborhood
canonicalize_neighborhood(bim::game::cell_neighborhood n)
{
  // If there is no neighbor on the left, we don't care if there is a neighbor
  // on the top-left and bottom-left corner. The left edge covers the corners.
  if (!(n & bim::game::cell_neighborhood::left))
    n &= ~(bim::game::cell_neighborhood::down_left
           | bim::game::cell_neighborhood::up_left);

  if (!(n & bim::game::cell_neighborhood::right))
    n &= ~(bim::game::cell_neighborhood::down_right
           | bim::game::cell_neighborhood::up_right);

  if (!(n & bim::game::cell_neighborhood::up))
    n &= ~(bim::game::cell_neighborhood::up_left
           | bim::game::cell_neighborhood::up_right);

  if (!(n & bim::game::cell_neighborhood::down))
    n &= ~(bim::game::cell_neighborhood::down_left
           | bim::game::cell_neighborhood::down_right);

  return n;
}

static void neighborhood_to_string(char* buffer,
                                   bim::game::cell_neighborhood n)
{
  const int s = sizeof(n) * 8;
  const bim::game::cell_neighborhood mask =
      (bim::game::cell_neighborhood)(1 << (s - 1));

  for (int i = 0; i != s; ++i)
    buffer[i] = '0' + bool(n & (mask >> i));
}

bim::axmol::app::fog_display::fog_display(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
{
  alloc_assets(
      m_sprite, m_context.get_widget_context(),
      bim::game::g_default_arena_width * bim::game::g_default_arena_height,
      *style.get_declaration("fog-of-war"), *m_controls->fog_container);

  ax::SpriteFrameCache& sprite_frames = *ax::SpriteFrameCache::getInstance();

  for (int i = 0;; ++i)
    {
      const iscool::optional<const std::string&> frame_name =
          style.get_string("roll-in-frame." + std::to_string(i));

      if (!frame_name)
        break;

      m_roll_in_frame.emplace_back(
          sprite_frames.getSpriteFrameByName(*frame_name));
    }

  std::string sprite_frame_name = "sprites/fog/";
  int name_offset = sprite_frame_name.size();
  sprite_frame_name += "00000000.png";
  char* const neighborhood_string = &sprite_frame_name[name_offset];

  for (int i = 0; i != bim::game::cell_neighborhood_layout_count; ++i)
    {
      neighborhood_to_string(
          neighborhood_string,
          canonicalize_neighborhood((bim::game::cell_neighborhood)i));
      m_sprite_frame[i] = bim::axmol::ref_ptr<ax::SpriteFrame>(
          sprite_frames.getSpriteFrameByName(sprite_frame_name));
      assert(m_sprite_frame[i] != nullptr);
    }
}

bim::axmol::app::fog_display::~fog_display() = default;

const bim::axmol::widget::named_node_group&
bim::axmol::app::fog_display::display_nodes() const
{
  return m_controls->all_nodes;
}

void bim::axmol::app::fog_display::attached(
    const arena_display_config& display_config)
{
  m_display_config = display_config;

  const ax::Vec2 size(m_display_config.block_size,
                      m_display_config.block_size);

  for (ax::Sprite* s : m_sprite)
    s->setContentSize(size);
}

void bim::axmol::app::fog_display::displaying(
    std::uint8_t displayed_player_index)
{
  m_displayed_player = displayed_player_index;

  for (ax::Sprite* s : m_sprite)
    s->setVisible(false);
}

void bim::axmol::app::fog_display::update(const bim::game::contest& contest)
{
  ZoneScoped;

  const entt::registry& registry = contest.registry();

  // Ensure that the displayed player is still there, and pick another player
  // otherwise.
  std::uint8_t player_index = 0;

  for (const auto& [_, p] : registry.view<bim::game::player>().each())
    if (p.index == m_displayed_player)
      {
        player_index = p.index;
        break;
      }
    else
      player_index = std::max(player_index, p.index);

  const float sprite_opacity =
      (player_index == m_displayed_player) ? 255 : 128;

  std::size_t asset_index = 0;

  const bim::table_2d<bim::game::fog_of_war*>& fog_map =
      contest.fog_map(player_index);

  for (std::size_t y = 0, h = fog_map.height(); y != h; ++y)
    for (std::size_t x = 0, w = fog_map.width(); x != w; ++x)
      {
        const bim::game::fog_of_war* const fog = fog_map(x, y);

        if (!fog)
          continue;

        ax::Sprite& s = *m_sprite[asset_index];

        if (fog->opacity == 0)
          {
            s.setVisible(false);
            ++asset_index;
            continue;
          }

        s.setVisible(true);
        s.setOpacity(sprite_opacity);
        s.setPosition(
            m_display_config.grid_position_to_displayed_block_center(x, y));

        if (fog->opacity == bim::game::fog_of_war::full_opacity)
          s.setSpriteFrame(
              m_sprite_frame[(std::size_t)fog->neighborhood].get());
        else
          s.setSpriteFrame(
              m_roll_in_frame[m_roll_in_frame.size() * fog->opacity
                              / bim::game::fog_of_war::full_opacity]
                  .get());

        ++asset_index;
      }

  bim::axmol::widget::hide_while_visible(m_sprite, asset_index);
}

void bim::axmol::app::fog_display::closing()
{}
