#include <bim/axmol/app/screen/online_game.hpp>

#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/soft_stick.hpp>

#include <bim/net/contest_runner.hpp>
#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/exchange/game_update_exchange.hpp>
#include <bim/net/session_handler.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/brick_wall.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_kind.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/player_action.hpp>

#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/time/monotonic_now.hpp>

#define x_widget_scope bim::axmol::app::online_game::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Node, arena) x_widget(bim::axmol::widget::soft_stick, stick)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <axmol/2d/Sprite.h>
#include <axmol/base/Director.h>

#include <entt/entity/registry.hpp>

#include <cassert>

IMPLEMENT_SIGNAL(bim::axmol::app::online_game, game_over, m_game_over);

bim::axmol::app::online_game::online_game(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_flame_center_asset_name(*style.get_string("flame-center-asset-name"))
  , m_flame_arm_asset_name(*style.get_string("flame-arm-asset-name"))
  , m_flame_end_asset_name(*style.get_string("flame-end-asset-name"))
  , m_arena_width_in_blocks(*style.get_number("arena-width-in-blocks"))
{
  m_inputs.push_back(m_controls->stick->input_node());

  const bim::axmol::widget::context& widget_context =
      m_context.get_widget_context();

  constexpr int default_width = 13;
  constexpr int default_height = 15;
  constexpr int max_players = 4;
  constexpr int max_bombs_per_player = 8;

  m_players.resize(max_players);

  for (int i = 0; i != max_players; ++i)
    {
      const iscool::style::declaration& player_style =
          *style.get_declaration("player-" + std::to_string(i + 1));

      const bim::axmol::ref_ptr<ax::Sprite> widget =
          bim::axmol::widget::factory<ax::Sprite>::create(widget_context,
                                                          player_style);
      widget->setVisible(false);
      m_players[i] = widget.get();
      m_controls->arena->addChild(m_players[i]);
    }

  alloc_assets(m_walls, widget_context, default_width * default_height,
               *style.get_declaration("wall"));
  alloc_assets(m_brick_walls, widget_context,
               (default_width - 2) * (default_height - 2),
               *style.get_declaration("brick-wall"));
  alloc_assets(m_flames, widget_context,
               (default_width - 2) * (default_height - 2),
               *style.get_declaration("flame"));
  alloc_assets(m_bombs, widget_context, max_players * max_bombs_per_player,
               *style.get_declaration("bomb"));
}

bim::axmol::app::online_game::~online_game() = default;

bim::axmol::input::node_reference
bim::axmol::app::online_game::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::online_game::nodes() const
{
  return m_controls->all_nodes;
}

void bim::axmol::app::online_game::attached()
{
  m_arena_view_size = m_controls->arena->getContentSize();
  m_block_size = m_arena_view_size.x / m_arena_width_in_blocks;

  const auto resize_to_block_width =
      [this](const std::vector<ax::Sprite*>& sprites)
  {
    assert(!sprites.empty());

    const ax::Vec2 sprite_size = sprites[0]->getContentSize();
    const float height_ratio = sprite_size.y / sprite_size.x;
    const ax::Vec2 size(m_block_size, m_block_size * height_ratio);

    for (ax::Sprite* s : sprites)
      s->setContentSize(size);
  };

  resize_to_block_width(m_players);
  resize_to_block_width(m_walls);
  resize_to_block_width(m_brick_walls);
  resize_to_block_width(m_flames);
  resize_to_block_width(m_bombs);
}

void bim::axmol::app::online_game::displaying(
    const bim::net::game_launch_event& event)
{
  m_contest.reset(
      new bim::game::contest(1234, 80, event.player_count, 13, 15));
  m_game_channel.reset(new iscool::net::message_channel(
      m_context.get_session_handler()->message_stream(),
      m_context.get_session_handler()->session_id(), event.channel));
  m_update_exchange.reset(
      new bim::net::game_update_exchange(*m_game_channel, event.player_count));
  m_contest_runner.reset(new bim::net::contest_runner(
      *m_contest, *m_update_exchange, event.player_index, event.player_count));

  m_local_player_index = event.player_index;

  // Display as many assets as needed.
  const bim::game::arena& arena = m_contest->arena();
  const int arena_width = arena.width();
  const int arena_height = arena.height();
  std::size_t asset_index;

  // Players.
  for (asset_index = 0; asset_index != event.player_count; ++asset_index)
    m_players[asset_index]->setVisible(true);

  for (; asset_index != m_players.size(); ++asset_index)
    m_players[asset_index]->setVisible(false);

  // Static walls.
  asset_index = 0;

  for (int y = 0; y != arena_height; ++y)
    for (int x = 0; (x != arena_width) && (asset_index != m_walls.size()); ++x)
      if (arena.is_static_wall(x, y))
        {
          ax::Sprite& s = *m_walls[asset_index];
          s.setPosition(grid_position_to_displayed_block_center(x, y));
          s.setVisible(true);
          ++asset_index;
        }

  for (; asset_index != m_walls.size(); ++asset_index)
    m_walls[asset_index]->setVisible(false);

  display_brick_walls();
}

void bim::axmol::app::online_game::displayed()
{
  m_update_exchange->connect_to_started(
      [this]() -> void
      {
        schedule_tick();
      });
  m_update_exchange->start();
}

void bim::axmol::app::online_game::closing()
{}

void bim::axmol::app::online_game::schedule_tick()
{
  const std::chrono::nanoseconds update_interval =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::duration<float>(
              ax::Director::getInstance()->getAnimationInterval()));

  m_last_tick_date = iscool::time::monotonic_now<std::chrono::nanoseconds>();
  // TODO: The actual delay between this call and the actual call to tick is
  // twice the update_interval. Why?
  m_tick_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        tick();
      },
      update_interval);
}

void bim::axmol::app::online_game::tick()
{
  const std::chrono::duration now =
      iscool::time::monotonic_now<std::chrono::nanoseconds>();
  const bim::game::contest_result result =
      m_contest_runner->run(now - m_last_tick_date);
  m_last_tick_date = now;

  apply_inputs();
  refresh_display();

  if (result.still_running())
    schedule_tick();
  else
    {
      stop();
      m_game_over(result);
    }
}

template <typename T>
void bim::axmol::app::online_game::alloc_assets(
    std::vector<T*>& out, const bim::axmol::widget::context& context,
    std::size_t count, const iscool::style::declaration& style) const
{
  ax::Node& arena = *m_controls->arena;

  out.resize(count);

  for (T*& p : out)
    {
      const bim::axmol::ref_ptr<T> widget =
          bim::axmol::widget::factory<T>::create(context, style);
      p = widget.get();
      p->setVisible(false);
      arena.addChild(p);
    }
}

void bim::axmol::app::online_game::apply_inputs() const
{
  bim::game::player_action* player_action =
      bim::game::find_player_action_by_index(m_contest->registry(),
                                             m_local_player_index);

  if ((player_action == nullptr) || player_action->full())
    return;

  const ax::Vec2& drag = m_controls->stick->drag();

  constexpr float move_threshold = 0.1;

  if (drag.x >= move_threshold)
    player_action->push(bim::game::player_action_kind::right);
  else if (drag.x <= -move_threshold)
    player_action->push(bim::game::player_action_kind::left);

  if (player_action->full())
    return;

  if (drag.y >= move_threshold)
    player_action->push(bim::game::player_action_kind::up);
  else if (drag.y <= -move_threshold)
    player_action->push(bim::game::player_action_kind::down);
}

void bim::axmol::app::online_game::refresh_display() const
{
  display_brick_walls();
  display_players();
  display_bombs();
  display_flames();
}

void bim::axmol::app::online_game::display_brick_walls() const
{
  const entt::registry& registry = m_contest->registry();
  std::size_t asset_index = 0;

  registry.view<bim::game::position_on_grid, bim::game::brick_wall>().each(
      [this, &asset_index](const bim::game::position_on_grid& p) -> void
      {
        ax::Sprite& s = *m_brick_walls[asset_index];

        s.setVisible(true);
        s.setPosition(grid_position_to_displayed_block_center(p.x, p.y));
        ++asset_index;
      });

  for (std::size_t n = m_brick_walls.size(); asset_index != n; ++asset_index)
    {
      if (m_brick_walls[asset_index]->isVisible())
        m_brick_walls[asset_index]->setVisible(false);
      else
        break;
    }
}

void bim::axmol::app::online_game::display_players() const
{
  const entt::registry& registry = m_contest->registry();

  for (ax::Sprite* s : m_players)
    s->setVisible(false);

  registry.view<bim::game::player, bim::game::fractional_position_on_grid>()
      .each(
          [this](const bim::game::player& player,
                 const bim::game::fractional_position_on_grid& p) -> void
          {
            ax::Sprite& s = *m_players[player.index];

            s.setVisible(true);
            s.setPosition(grid_position_to_display(p.x_float(), p.y_float()));
          });
}

void bim::axmol::app::online_game::display_bombs() const
{
  const entt::registry& registry = m_contest->registry();
  std::size_t asset_index = 0;

  registry.view<bim::game::position_on_grid, bim::game::bomb>().each(
      [this, &asset_index](const bim::game::position_on_grid& p,
                           const bim::game::bomb& b) -> void
      {
        ax::Sprite& s = *m_bombs[asset_index];

        s.setVisible(true);
        s.setPosition(grid_position_to_displayed_block_center(p.x, p.y));

        const size_t f =
            (b.duration_until_explosion > std::chrono::seconds(1)) ? 300 : 100;

        if (b.duration_until_explosion.count() / f % 2 == 0)
          s.setScale(1.1);
        else
          s.setScale(1);

        ++asset_index;
      });

  for (std::size_t n = m_bombs.size(); asset_index != n; ++asset_index)
    if (m_bombs[asset_index]->isVisible())
      m_bombs[asset_index]->setVisible(false);
    else
      break;
}

void bim::axmol::app::online_game::display_flames() const
{
  const entt::registry& registry = m_contest->registry();
  std::size_t asset_index = 0;

  registry.view<bim::game::position_on_grid, bim::game::flame>().each(
      [this, &asset_index](const bim::game::position_on_grid& p,
                           const bim::game::flame& f) -> void
      {
        ax::Sprite& s = *m_flames[asset_index];
        // Changing the sprite frame changes the sprite size, thus we must
        // restore it ourselves.
        ax::Vec2 size = s.getContentSize();

        s.setVisible(true);
        s.setPosition(grid_position_to_displayed_block_center(p.x, p.y));

        switch (f.segment)
          {
          case bim::game::flame_segment::origin:
            s.setSpriteFrame(m_flame_center_asset_name);
            s.setRotation(0);
            break;
          case bim::game::flame_segment::arm:
            {
              s.setSpriteFrame(m_flame_arm_asset_name);

              if (bim::game::is_vertical(f.direction))
                s.setRotation(90);
              else
                s.setRotation(0);
              break;
            }
          case bim::game::flame_segment::tip:
            {
              s.setSpriteFrame(m_flame_end_asset_name);
              switch (f.direction)
                {
                case bim::game::flame_direction::right:
                  s.setRotation(0);
                  break;
                case bim::game::flame_direction::down:
                  s.setRotation(90);
                  break;
                case bim::game::flame_direction::left:
                  s.setRotation(180);
                  break;
                case bim::game::flame_direction::up:
                  s.setRotation(270);
                  break;
                }
            }
          }

        s.setContentSize(size);
        ++asset_index;
      });

  for (std::size_t n = m_flames.size(); asset_index != n; ++asset_index)
    if (m_flames[asset_index]->isVisible())
      m_flames[asset_index]->setVisible(false);
    else
      break;
}

ax::Vec2 bim::axmol::app::online_game::grid_position_to_displayed_block_center(
    std::uint8_t x, std::uint8_t y) const
{
  const float center_x = (float)x * m_block_size + m_block_size / 2;
  const float center_y =
      m_arena_view_size.y - m_block_size / 2 - (float)y * m_block_size;

  return ax::Vec2(center_x, center_y);
}

ax::Vec2 bim::axmol::app::online_game::grid_position_to_display(float x,
                                                                float y) const
{
  const float view_x = x * m_block_size;
  const float view_y = m_arena_view_size.y - y * m_block_size;

  return ax::Vec2(view_x, view_y);
}

void bim::axmol::app::online_game::stop()
{
  m_contest_runner.reset();
  m_update_exchange.reset();
  m_game_channel.reset();
  m_contest.reset();
}
