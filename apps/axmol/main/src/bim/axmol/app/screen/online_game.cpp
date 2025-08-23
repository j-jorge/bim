// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/online_game.hpp>

#include <bim/axmol/app/alloc_assets.hpp>
#include <bim/axmol/app/fog_display.hpp>
#include <bim/axmol/app/player_progress_tracker.hpp>
#include <bim/axmol/app/preference/controls.hpp>
#include <bim/axmol/app/widget/player.hpp>

#include <bim/axmol/widget/animation/animation_cache.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/hide_while_visible.hpp>
#include <bim/axmol/widget/merge_named_node_groups.hpp>
#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/button.hpp>
#include <bim/axmol/widget/ui/peephole.hpp>
#include <bim/axmol/widget/ui/soft_pad.hpp>
#include <bim/axmol/widget/ui/soft_stick.hpp>
#include <bim/axmol/widget/ui/tiling.hpp>

#include <bim/net/contest_runner.hpp>
#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/exchange/game_update_exchange.hpp>
#include <bim/net/session_handler.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/brick_wall.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/game_timer.hpp>
#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/constant/default_arena_size.hpp>
#include <bim/game/constant/falling_block_duration.hpp>
#include <bim/game/constant/max_bomb_count_per_player.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/contest_fingerprint.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/level_generation.hpp>
#include <bim/game/player_action.hpp>
#include <bim/game/static_wall.hpp>

#include <bim/assume.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/system/haptic_feedback.hpp>
#include <iscool/time/monotonic_now.hpp>

#include <axmol/2d/Label.h>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/keyboard_gamepad.hpp>

#define x_widget_scope bim::axmol::app::online_game::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Node, arena) x_widget(ax::Node, control_panel)                 \
      x_widget(bim::axmol::widget::soft_stick, joystick)                      \
          x_widget(bim::axmol::widget::soft_pad, directional_pad)             \
              x_widget(bim::axmol::widget::button, bomb_button)               \
                  x_widget(ax::Label, timer)                                  \
                      x_widget(bim::axmol::widget::peephole, peephole)        \
                          x_widget(bim::axmol::widget::tiling, background)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <axmol/2d/Sprite.h>
#include <axmol/base/Director.h>
#include <axmol/renderer/Shaders.h>
#include <axmol/renderer/backend/ProgramManager.h>
#include <axmol/renderer/backend/ProgramState.h>

#include <entt/entity/registry.hpp>

#include <fmt/format.h>

#include <cassert>

template <typename T>
static void hide_all(std::vector<T*> nodes)
{
  for (T* const n : nodes)
    n->setVisible(false);
}

static void alloc_assets(std::vector<ax::Sprite*>& out,
                         const bim::axmol::widget::context& context,
                         std::size_t count,
                         const iscool::style::declaration& style,
                         ax::Node& parent)
{
  bim::axmol::app::alloc_assets(out, context, count, style, parent);
  hide_all(out);
}

IMPLEMENT_SIGNAL(bim::axmol::app::online_game, game_over, m_game_over)

bim::axmol::app::online_game::online_game(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_keyboard_gamepad(ax::EventKeyboard::KeyCode::KEY_UP_ARROW,
                       ax::EventKeyboard::KeyCode::KEY_DOWN_ARROW,
                       ax::EventKeyboard::KeyCode::KEY_RIGHT_ARROW,
                       ax::EventKeyboard::KeyCode::KEY_LEFT_ARROW,
                       ax::EventKeyboard::KeyCode::KEY_SPACE)
  , m_style_pad_on_the_left(*style.get_declaration("bounds.d-pad-on-the-left"))
  , m_style_pad_on_the_right(
        *style.get_declaration("bounds.d-pad-on-the-right"))
  , m_style_use_joystick(*style.get_declaration("display.use-joystick"))
  , m_style_use_d_pad(*style.get_declaration("display.use-d-pad"))
  , m_style_player{ &*style.get_declaration("display.player-1"),
                    &*style.get_declaration("display.player-2"),
                    &*style.get_declaration("display.player-3"),
                    &*style.get_declaration("display.player-4") }
  , m_animation_cache(new bim::axmol::widget::animation_cache())
  , m_fog(new fog_display(context, *style.get_declaration("fog-display")))
  , m_flame_center_asset_name(*style.get_string("flame-center-asset-name"))
  , m_flame_arm_asset_name(*style.get_string("flame-arm-asset-name"))
  , m_flame_end_asset_name(*style.get_string("flame-end-asset-name"))
  , m_arena_width_in_blocks(*style.get_number("arena-width-in-blocks"))
{
  m_animation_cache->load("animations.json"sv);

  m_nodes = m_controls->all_nodes;
  bim::axmol::widget::merge_named_node_groups(m_nodes, m_fog->display_nodes());

  // The control panel fills the space below the arena. Its size will be
  // adjusted when the screen is added in the scene.
  m_controls->control_panel->setAnchorPoint(ax::Vec2(0.5, 0));

  m_controls->peephole->connect_to_shown(
      [this]() -> void
      {
        m_update_exchange->start();
      });

  const auto request_drop_bomb = [this]()
  {
    m_bomb_drop_requested = true;
  };

  m_controls->bomb_button->connect_to_pressed(request_drop_bomb);
  m_controls->bomb_button->cancel_on_swipe(false);

  m_inputs.push_back(m_controls->bomb_button->input_node());
  m_inputs.push_back(m_controls->joystick->input_node());
  m_inputs.push_back(m_controls->directional_pad->input_node());
  m_inputs.push_back(m_keyboard_gamepad);

  m_keyboard_gamepad->connect_to_action(request_drop_bomb);

  m_controls->directional_pad->connect_to_pressed(
      [haptic = context.get_haptic_feedback()]() -> void
      {
        haptic->click();
      });

  const bim::axmol::widget::context& widget_context =
      m_context.get_widget_context();

  m_players.resize(bim::game::g_max_player_count);

  const iscool::style::declaration& player_style =
      *style.get_declaration("player");

  for (int i = 0; i != bim::game::g_max_player_count; ++i)
    {
      m_players[i] = bim::axmol::widget::factory<player>::create(
                         m_context.get_widget_context(), player_style)
                         .get();

      m_players[i]->setVisible(false);
      m_controls->arena->addChild(m_players[i]);
    }

  constexpr int inner_width = bim::game::g_default_arena_width - 2;
  constexpr int inner_height = bim::game::g_default_arena_height - 2;

  ::alloc_assets(m_walls[(std::size_t)bim::game::cell_neighborhood::none],
                 widget_context, inner_width * inner_height,
                 *style.get_declaration("wall"), *m_controls->arena);
  ::alloc_assets(m_walls[(std::size_t)(bim::game::cell_neighborhood::all
                                       & ~bim::game::cell_neighborhood::down)],
                 widget_context, inner_width,
                 *style.get_declaration("arena-border.top"),
                 *m_controls->arena);
  ::alloc_assets(m_walls[(std::size_t)(bim::game::cell_neighborhood::all
                                       & ~bim::game::cell_neighborhood::up)],
                 widget_context, inner_width,
                 *style.get_declaration("arena-border.bottom"),
                 *m_controls->arena);
  ::alloc_assets(
      m_walls[(std::size_t)(bim::game::cell_neighborhood::all
                            & ~bim::game::cell_neighborhood::right)],
      widget_context, inner_height,
      *style.get_declaration("arena-border.left"), *m_controls->arena);
  ::alloc_assets(m_walls[(std::size_t)(bim::game::cell_neighborhood::all
                                       & ~bim::game::cell_neighborhood::left)],
                 widget_context, inner_height,
                 *style.get_declaration("arena-border.right"),
                 *m_controls->arena);
  ::alloc_assets(
      m_walls[(std::size_t)(bim::game::cell_neighborhood::all
                            & ~bim::game::cell_neighborhood::down_right)],
      widget_context, 1, *style.get_declaration("arena-border.top-left"),
      *m_controls->arena);
  ::alloc_assets(
      m_walls[(std::size_t)(bim::game::cell_neighborhood::all
                            & ~bim::game::cell_neighborhood::down_left)],
      widget_context, 1, *style.get_declaration("arena-border.top-right"),
      *m_controls->arena);
  ::alloc_assets(
      m_walls[(std::size_t)(bim::game::cell_neighborhood::all
                            & ~bim::game::cell_neighborhood::up_right)],
      widget_context, 1, *style.get_declaration("arena-border.bottom-left"),
      *m_controls->arena);
  ::alloc_assets(
      m_walls[(std::size_t)(bim::game::cell_neighborhood::all
                            & ~bim::game::cell_neighborhood::up_left)],
      widget_context, 1, *style.get_declaration("arena-border.bottom-right"),
      *m_controls->arena);

  constexpr int free_cell_count =
      (inner_width * inner_height) - (inner_width * inner_height) / 4;
  ::alloc_assets(m_falling_blocks, widget_context, free_cell_count,
                 *style.get_declaration("falling-block"), *m_controls->arena);
  ::alloc_assets(m_falling_blocks_shadows, widget_context, free_cell_count,
                 *style.get_declaration("falling-block-shadow"),
                 *m_controls->arena);
  ::alloc_assets(m_brick_walls, widget_context, free_cell_count,
                 *style.get_declaration("brick-wall"), *m_controls->arena);
  ::alloc_assets(m_bombs, widget_context,
                 bim::game::g_max_player_count
                     * bim::game::g_max_bomb_count_per_player,
                 *style.get_declaration("bomb"), *m_controls->arena);
  ::alloc_assets(m_flames, widget_context, free_cell_count,
                 *style.get_declaration("flame"), *m_controls->arena);
  ::alloc_assets(m_bomb_power_ups, widget_context,
                 bim::game::g_bomb_power_up_count_in_level,
                 *style.get_declaration("power-up-bomb"), *m_controls->arena);
  ::alloc_assets(m_flame_power_ups, widget_context,
                 bim::game::g_flame_power_up_count_in_level,
                 *style.get_declaration("power-up-flame"), *m_controls->arena);
  ::alloc_assets(m_invisibility_power_ups, widget_context,
                 bim::game::g_invisibility_power_up_count_in_level,
                 *style.get_declaration("power-up-invisibility"),
                 *m_controls->arena);
  ::alloc_assets(m_shield_power_ups, widget_context,
                 bim::game::g_shield_power_up_count_in_level,
                 *style.get_declaration("power-up-shield"),
                 *m_controls->arena);

  ax::backend::Program* const shine =
      ax::backend::ProgramManager::getInstance()->loadProgram(
          ax::positionTextureColor_vert, "shaders/shine_fs");

  create_power_up_shader(m_bomb_power_ups, *shine);
  create_power_up_shader(m_flame_power_ups, *shine);
  create_power_up_shader(m_invisibility_power_ups, *shine);
  create_power_up_shader(m_shield_power_ups, *shine);
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
  return m_nodes;
}

void bim::axmol::app::online_game::attached()
{
  ax::Node& arena = *m_controls->arena;
  const ax::Vec2 arena_view_size = arena.getContentSize();
  const ax::Vec2 arena_anchor_point = arena.getAnchorPoint();

  // Adjust the control panel to fill the space below the arena.
  m_controls->control_panel->setPosition(
      ax::Vec2(arena.getPosition().x - arena_anchor_point.x * arena_view_size.x
                   + arena_view_size.x / 2,
               0));
  m_controls->control_panel->setContentSize(ax::Vec2(
      arena_view_size.x,
      arena.getPosition().y - arena_anchor_point.y * arena_view_size.y));

  const float block_size = arena_view_size.x / m_arena_width_in_blocks;
  m_display_config = arena_display_config{ .block_size = block_size,
                                           .view_height = arena_view_size.y };

  for (const std::vector<ax::Sprite*>& walls : m_walls)
    resize_to_block_width(walls);

  resize_to_block_width(m_players);
  resize_to_block_width(m_falling_blocks);
  resize_to_block_width(m_falling_blocks_shadows);
  resize_to_block_width(m_brick_walls);
  resize_to_block_width(m_flames);
  resize_to_block_width(m_bombs);
  resize_to_block_width(m_bomb_power_ups);
  resize_to_block_width(m_flame_power_ups);
  resize_to_block_width(m_invisibility_power_ups);
  resize_to_block_width(m_shield_power_ups);

  m_fog->attached(m_display_config);
}

void bim::axmol::app::online_game::displaying(
    const bim::net::game_launch_event& event)
{
  m_bomb_drop_requested = false;
  m_controls->bomb_button->enable(false);
  m_keyboard_gamepad->enable(false);

  configure_direction_pad();

  bim::axmol::widget::apply_display(
      m_context.get_widget_context().style_cache, m_controls->all_nodes,
      *m_style_player[std::min<std::size_t>(event.player_index,
                                            m_style_player.size())]);

  m_contest.reset(
      new bim::game::contest(event.fingerprint, event.player_index));
  m_game_channel.reset(new iscool::net::message_channel(
      m_context.get_session_handler()->message_stream(),
      m_context.get_session_handler()->session_id(), event.channel));

  const int player_count = event.fingerprint.player_count;
  const bim::game::player_animations& player_animations =
      m_contest->context().get<const bim::game::player_animations>();

  for (int i = 0; i != player_count; ++i)
    m_players[i]->configure(*m_animation_cache, player_animations, i);

  m_update_exchange.reset(
      new bim::net::game_update_exchange(*m_game_channel, player_count));
  m_update_exchange->connect_to_started(
      [this]() -> void
      {
        m_controls->peephole->reveal();
        m_controls->bomb_button->enable(true);
        m_keyboard_gamepad->enable(true);
        m_last_tick_date =
            iscool::time::monotonic_now<std::chrono::nanoseconds>();
        m_game_start_date = m_last_tick_date;
        schedule_tick();
      });
  m_contest_runner.reset(new bim::net::contest_runner(
      *m_contest, *m_update_exchange, event.player_index, player_count));

  m_local_player_index = event.player_index;

  m_z_order.resize(event.fingerprint.arena_height);

  // Hide all assets
  hide_all(m_players);
  hide_all(m_falling_blocks);
  hide_all(m_falling_blocks_shadows);
  hide_all(m_brick_walls);
  hide_all(m_bombs);
  hide_all(m_flames);
  hide_all(m_bomb_power_ups);
  hide_all(m_flame_power_ups);
  hide_all(m_invisibility_power_ups);
  hide_all(m_shield_power_ups);
  m_fog->displaying(m_local_player_index);

  reset_z_order();
  display_brick_walls();
  display_players();
  display_static_walls();
  m_fog->update(*m_contest);

  m_controls->background->set_node_width_in_tiles(
      event.fingerprint.arena_width);

  const ax::Node& player = *m_players[m_local_player_index];
  m_controls->peephole->prepare(
      player.convertToWorldSpace(player.getContentSize() / 2));

  m_controls->timer->setString("BIM!");
}

void bim::axmol::app::online_game::displayed()
{
  m_controls->peephole->show();
}

void bim::axmol::app::online_game::closing()
{
  stop();
}

void bim::axmol::app::online_game::configure_direction_pad()
{
  const bool pad_on_the_left =
      direction_pad_on_the_left(*m_context.get_local_preferences());

  bim::axmol::widget::apply_bounds(
      m_context.get_widget_context(), m_controls->all_nodes,
      pad_on_the_left ? m_style_pad_on_the_left : m_style_pad_on_the_right);
  m_use_stick =
      direction_pad_kind_is_stick(*m_context.get_local_preferences());

  if (m_use_stick)
    m_controls->joystick->set_layout_on_the_left(pad_on_the_left);

  bim::axmol::widget::apply_display(
      m_context.get_widget_context().style_cache, m_controls->all_nodes,
      m_use_stick ? m_style_use_joystick : m_style_use_d_pad);
}

void bim::axmol::app::online_game::create_power_up_shader(
    std::vector<ax::Sprite*>& sprites, ax::backend::Program& p)
{
  bim_assume(!sprites.empty());

  ax::backend::ProgramState* ps = new ax::backend::ProgramState(&p);
  ps->autorelease();

  const ax::Sprite& ref = *sprites[0];

  {
    const ax::backend::UniformLocation location =
        ps->getUniformLocation("sprite_rect");
    const ax::Rect sprite_rect = ref.getTextureRect();
    const ax::Vec4 v4(sprite_rect.origin.x, sprite_rect.origin.y,
                      sprite_rect.size.width, sprite_rect.size.height);
    ps->setUniform(location, &v4, sizeof(v4));
  }

  {
    const ax::backend::UniformLocation location =
        ps->getUniformLocation("texture_size");
    const ax::Vec2 texture_size = ref.getTexture()->getContentSize();
    ps->setUniform(location, &texture_size, sizeof(texture_size));
  }

  ps->updateBatchId();

  for (ax::Sprite* s : sprites)
    s->setProgramState(ps);

  m_shader_programs.push_back(ps);
  m_time_uniform.push_back(ps->getUniformLocation("time"));
}

void bim::axmol::app::online_game::schedule_tick()
{
  const std::chrono::nanoseconds update_interval =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::duration<float>(
              ax::Director::getInstance()->getAnimationInterval()));

  m_tick_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        tick();
      },
      update_interval);
}

void bim::axmol::app::online_game::tick()
{
  const std::chrono::nanoseconds now =
      iscool::time::monotonic_now<std::chrono::nanoseconds>();
  const std::chrono::nanoseconds runner_step = now - m_last_tick_date;

  update_shader_time(now);

  std::chrono::nanoseconds final_step;

  const int ticks_ahead =
      m_contest_runner->local_tick() - m_contest_runner->confirmed_tick();

  const int max_ticks_ahead = bim::game::player_action_queue::queue_size;

  if (ticks_ahead <= max_ticks_ahead)
    final_step = runner_step;
  else
    {
      // Slow the game if the player is far ahead the shared state.
      const int delta_tick = ticks_ahead - max_ticks_ahead;
      const std::chrono::nanoseconds adjusted_step =
          runner_step - runner_step * delta_tick / (3 * max_ticks_ahead);

      final_step =
          std::max(2 * runner_step / 3, std::min(adjusted_step, runner_step));
    }

  const bim::game::contest_result result = m_contest_runner->run(final_step);

  m_last_tick_date = now;

  apply_inputs();
  refresh_display();

  if (result.still_running())
    schedule_tick();
  else
    {
      stop();
      m_context.get_player_progress_tracker()->game_over_in_public_arena(
          result, m_local_player_index);
      m_game_over(result);
    }
}

void bim::axmol::app::online_game::update_shader_time(
    std::chrono::nanoseconds now) const
{
  const std::chrono::nanoseconds dt = now - m_game_start_date;
  const float t =
      std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();

  for (size_t i = 0, n = m_shader_programs.size(); i != n; ++i)
    m_shader_programs[i]->setUniform(m_time_uniform[i], &t, sizeof(t));
}

template <typename T>
void bim::axmol::app::online_game::resize_to_block_width(
    const std::vector<T*>& nodes) const
{
  resize_to_block_width(std::span<T* const>(nodes));
}

template <typename T>
void bim::axmol::app::online_game::resize_to_block_width(
    std::span<T* const> nodes) const
{
  if (nodes.empty())
    return;

  const ax::Vec2 initial_size = nodes[0]->getContentSize();
  const float height_ratio = initial_size.y / initial_size.x;
  const ax::Vec2 size(m_display_config.block_size,
                      m_display_config.block_size * height_ratio);

  for (T* n : nodes)
    n->setContentSize(size);
}

void bim::axmol::app::online_game::apply_inputs()
{
  bim::game::player_action* player_action =
      bim::game::find_player_action_by_index(m_contest->registry(),
                                             m_local_player_index);

  if (player_action == nullptr)
    return;

  const ax::Vec2 input_direction[] = {
    m_controls->joystick->drag(), m_controls->directional_pad->direction(),
    ax::Vec2(m_keyboard_gamepad->horizontal(), m_keyboard_gamepad->vertical())
  };

  ax::Vec2 drag = {};

  for (const ax::Vec2& d : input_direction)
    if (d != ax::Vec2::ZERO)
      {
        drag = d;
        break;
      }

  const float abs_x = std::abs(drag.x);
  const float abs_y = std::abs(drag.y);
  constexpr float move_threshold = 0.1;
  const float dx = (abs_x >= move_threshold) ? drag.x : 0;
  const float dy = (abs_y >= move_threshold) ? drag.y : 0;

  // Move either horizontally or vertically, but not both, as it tend to
  // produce unexpected turns on the grid layout of the game.
  if ((dx != 0) && (abs_x >= abs_y))
    {
      if (dx >= 0)
        player_action->movement = bim::game::player_movement::right;
      else
        player_action->movement = bim::game::player_movement::left;
    }
  else if (dy != 0)
    {
      if (dy >= 0)
        player_action->movement = bim::game::player_movement::up;
      else if (dy <= 0)
        player_action->movement = bim::game::player_movement::down;
    }

  if (m_bomb_drop_requested)
    {
      player_action->drop_bomb = true;
      m_bomb_drop_requested = false;
    }
}

void bim::axmol::app::online_game::reset_z_order()
{
  for (std::size_t i = 0, n = m_z_order.size(); i != n; ++i)
    m_z_order[i] = i * 1000;
}

void bim::axmol::app::online_game::refresh_display()
{
  reset_z_order();
  display_brick_walls();
  display_bombs();
  display_flames();
  display_power_ups<bim::game::bomb_power_up>(m_bomb_power_ups);
  display_power_ups<bim::game::flame_power_up>(m_flame_power_ups);
  display_power_ups<bim::game::invisibility_power_up>(
      m_invisibility_power_ups);
  display_power_ups<bim::game::shield_power_up>(m_shield_power_ups);
  display_players();

  display_static_walls();
  display_falling_blocks();
  m_fog->update(*m_contest);

  display_main_timer();
}

void bim::axmol::app::online_game::display_static_walls()
{
  const bim::game::arena& arena = m_contest->arena();
  std::array<std::size_t, bim::game::cell_neighborhood_layout_count>
      asset_index{};

  for (const bim::game::static_wall& wall : arena.static_walls())
    {
      const std::size_t n = (std::size_t)wall.neighbors;
      std::size_t& i = asset_index[n];

      if (i == m_walls[n].size())
        continue;

      display_at(wall.y, *m_walls[n][i],
                 m_display_config.grid_position_to_displayed_block_center(
                     wall.x, wall.y));
      ++i;
    }

  for (std::size_t i = 0; i != asset_index.size(); ++i)
    bim::axmol::widget::hide_while_visible(m_walls[i], asset_index[i]);
}

void bim::axmol::app::online_game::display_falling_blocks()
{
  const entt::registry& registry = m_contest->registry();
  std::size_t asset_index = 0;

  registry
      .view<bim::game::position_on_grid, bim::game::falling_block,
            bim::game::timer>()
      .each(
          [this, &asset_index](const bim::game::position_on_grid& p,
                               const bim::game::timer& t) -> void
          {
            const ax::Vec2 start =
                m_display_config.grid_position_to_displayed_block_center(
                    p.x, p.y - 1);
            const ax::Vec2 end =
                m_display_config.grid_position_to_displayed_block_center(p.x,
                                                                         p.y);
            const float remaining_ratio =
                std::chrono::duration_cast<std::chrono::duration<float>>(
                    t.duration)
                    .count()
                / std::chrono::duration_cast<std::chrono::duration<float>>(
                      bim::game::g_falling_block_duration)
                      .count();
            const ax::Vec2 display_position(
                start + (end - start) * (1 - remaining_ratio));

            display_at(p.y, *m_falling_blocks_shadows[asset_index], end);
            display_at(p.y, *m_falling_blocks[asset_index], display_position);
            ++asset_index;
          });

  bim::axmol::widget::hide_while_visible(m_falling_blocks, asset_index);
  bim::axmol::widget::hide_while_visible(m_falling_blocks_shadows,
                                         asset_index);
}

void bim::axmol::app::online_game::display_brick_walls()
{
  const entt::registry& registry = m_contest->registry();
  std::size_t asset_index = 0;

  registry.view<bim::game::position_on_grid, bim::game::brick_wall>().each(
      [this, &asset_index](const bim::game::position_on_grid& p) -> void
      {
        display_at(p.y, *m_brick_walls[asset_index],
                   m_display_config.grid_position_to_displayed_block_center(
                       p.x, p.y));
        ++asset_index;
      });

  bim::axmol::widget::hide_while_visible(m_brick_walls, asset_index);
}

void bim::axmol::app::online_game::display_players()
{
  const entt::registry& registry = m_contest->registry();

  bool local_still_alive = false;

  for (const auto& [_, p] : registry.view<bim::game::player>().each())
    if (p.index == m_local_player_index)
      {
        local_still_alive = true;
        break;
      }

  for (player* p : m_players)
    p->setVisible(false);

  registry
      .view<bim::game::player, bim::game::fractional_position_on_grid,
            bim::game::animation_state>()
      .each(
          [this,
           local_still_alive](entt::entity e, const bim::game::player& player,
                              const bim::game::fractional_position_on_grid& p,
                              const bim::game::animation_state& a) -> void
          {
            display_player(local_still_alive, e, player, p, a);
          });
}

void bim::axmol::app::online_game::display_player(
    bool local_still_alive, entt::entity e, const bim::game::player& player,
    const bim::game::fractional_position_on_grid& p,
    const bim::game::animation_state& a)
{
  const entt::registry& registry = m_contest->registry();
  bim::axmol::app::player& w = *m_players[player.index];

  display_at(
      p.grid_aligned_y(), w,
      m_display_config.grid_position_to_display(p.x_float(), p.y_float()));

  w.set_animation(bim::game::has_shield(registry, e), a);

  std::uint8_t opacity;

  const bool is_invisible = bim::game::is_invisible(registry, e);

  if (is_invisible)
    opacity =
        127 * (player.index == m_local_player_index || !local_still_alive);
  else
    {
      const bim::game::invincibility_state* const invincibility =
          registry.try_get<bim::game::invincibility_state>(e);

      if (invincibility)
        {
          const std::size_t ms = registry.storage<bim::game::timer>()
                                     ->get(invincibility->entity)
                                     .duration.count();
          opacity = (255 >> ((ms & (1 << 7)) >> 6));
        }
      else
        opacity = 255;
    }

  w.setOpacity(opacity);
}

void bim::axmol::app::online_game::display_bombs()
{
  const entt::registry& registry = m_contest->registry();
  std::size_t asset_index = 0;

  registry
      .view<bim::game::position_on_grid, bim::game::bomb, bim::game::timer>()
      .each(
          [this, &asset_index](const bim::game::position_on_grid& p,
                               const bim::game::bomb& b,
                               const bim::game::timer& t) -> void
          {
            ax::Sprite& s = *m_bombs[asset_index];

            display_at(
                p.y, s,
                m_display_config.grid_position_to_displayed_block_center(p.x,
                                                                         p.y));

            const size_t f =
                (t.duration > std::chrono::seconds(1)) ? 300 : 100;

            if (t.duration.count() / f % 2 == 0)
              s.setScale(1.1);
            else
              s.setScale(1);

            ++asset_index;
          });

  bim::axmol::widget::hide_while_visible(m_bombs, asset_index);
}

void bim::axmol::app::online_game::display_flames()
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

        display_at(p.y, s,
                   m_display_config.grid_position_to_displayed_block_center(
                       p.x, p.y));

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

  bim::axmol::widget::hide_while_visible(m_flames, asset_index);
}

template <typename T>
void bim::axmol::app::online_game::display_power_ups(
    const std::vector<ax::Sprite*>& assets)
{
  const entt::registry& registry = m_contest->registry();
  std::size_t asset_index = 0;

  registry.view<bim::game::position_on_grid, T>().each(
      [this, &assets,
       &asset_index](const bim::game::position_on_grid& p) -> void
      {
        display_at(p.y, *assets[asset_index],
                   m_display_config.grid_position_to_displayed_block_center(
                       p.x, p.y));
        ++asset_index;
      });

  bim::axmol::widget::hide_while_visible(assets, asset_index);
}

void bim::axmol::app::online_game::display_main_timer()
{
  const entt::registry& registry = m_contest->registry();

  registry.view<bim::game::timer, bim::game::game_timer>().each(
      [this](const bim::game::timer& t) -> void
      {
        const int duration_in_second =
            std::chrono::duration_cast<std::chrono::seconds>(t.duration)
                .count();

        const int seconds = duration_in_second % 60;
        const int minutes = duration_in_second / 60;

        m_controls->timer->setString(
            fmt::format("{:02}:{:02}", minutes, seconds));
      });
}

void bim::axmol::app::online_game::display_at(std::size_t arena_y,
                                              ax::Node& node,
                                              const ax::Vec2& position)
{
  node.setPosition(position);
  node.setVisible(true);

  assert(arena_y < m_z_order.size());
  node.setLocalZOrder(m_z_order[arena_y]);
  ++m_z_order[arena_y];
}

void bim::axmol::app::online_game::stop()
{
  m_contest_runner.reset();
  m_update_exchange.reset();
  m_game_channel.reset();
  m_contest.reset();
  m_tick_connection.disconnect();
}
