// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/game/cell_neighborhood.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <axmol/base/Types.h>

#include <entt/entity/fwd.hpp>

#include <boost/unordered/unordered_map.hpp>

#include <array>
#include <chrono>
#include <vector>

namespace bim::axmol::widget
{
  class context;
}

namespace bim::game
{
  class contest;
  class contest_result;
}

namespace bim::net
{
  class contest_runner;
  class game_launch_event;
  class game_update_exchange;
  class session_handler;
}

namespace ax
{
  class Sprite;
}

namespace iscool::net
{
  class message_channel;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::style
{
  class declaration;
}

namespace iscool::system
{
  class haptic_feedback;
}

namespace bim::axmol::app
{
  class online_game
  {
    DECLARE_SIGNAL(void(const bim::game::contest_result&), game_over,
                   m_game_over)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((bim::net::session_handler*)(session_handler))                //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((iscool::system::haptic_feedback*)(haptic_feedback))),
        ic_context_no_properties);

  public:
    online_game(const context& context,
                const iscool::style::declaration& style);
    ~online_game();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& nodes() const;

    void attached();
    void displaying(const bim::net::game_launch_event& event);
    void displayed();
    void closing();

  private:
    void configure_direction_pad();

    void schedule_tick();
    void tick();

    template <typename T>
    void alloc_assets(std::vector<T*>& out,
                      const bim::axmol::widget::context& context,
                      std::size_t count,
                      const iscool::style::declaration& style) const;

    void apply_inputs();

    void reset_z_order();
    void refresh_display();
    void display_static_walls();
    void display_falling_blocks();
    void display_brick_walls();
    void display_players();
    void display_bombs();
    void display_flames();
    void display_bomb_power_ups();
    void display_flame_power_ups();
    void display_main_timer();

    void display_at(std::size_t arena_y, ax::Node& node,
                    const ax::Vec2& position);

    ax::Vec2 grid_position_to_displayed_block_center(float x, float y) const;
    ax::Vec2 grid_position_to_display(float x, float y) const;

    void stop();

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 7);

    const iscool::style::declaration& m_style_pad_on_the_left;
    const iscool::style::declaration& m_style_pad_on_the_right;
    const iscool::style::declaration& m_style_use_joystick;
    const iscool::style::declaration& m_style_use_d_pad;

    iscool::signals::scoped_connection m_tick_connection;

    std::chrono::nanoseconds m_last_tick_date;
    std::unique_ptr<bim::game::contest> m_contest;
    std::unique_ptr<iscool::net::message_channel> m_game_channel;
    std::unique_ptr<bim::net::game_update_exchange> m_update_exchange;
    std::unique_ptr<bim::net::contest_runner> m_contest_runner;

    std::vector<ax::Sprite*> m_players;
    std::array<std::vector<ax::Sprite*>,
               (std::size_t)bim::game::cell_neighborhood::all>
        m_walls;
    std::vector<ax::Sprite*> m_falling_blocks;
    std::vector<ax::Sprite*> m_falling_blocks_shadows;
    std::vector<ax::Sprite*> m_brick_walls;
    std::vector<ax::Sprite*> m_bombs;
    std::vector<ax::Sprite*> m_flames;
    std::vector<ax::Sprite*> m_bomb_power_ups;
    std::vector<ax::Sprite*> m_flame_power_ups;

    std::vector<int> m_z_order;

    const std::string m_flame_center_asset_name;
    const std::string m_flame_arm_asset_name;
    const std::string m_flame_end_asset_name;

    const float m_arena_width_in_blocks;

    /// Width, and height, of a displayed block in the arena view.
    float m_block_size;

    /// The size of the node containing the arena.
    ax::Vec2 m_arena_view_size;
    std::uint8_t m_local_player_index;

    bool m_bomb_drop_requested;
    bool m_use_stick;
  };
}
