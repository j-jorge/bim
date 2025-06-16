// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/app/arena_display_config.hpp>

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
  class animation_cache;
  class context;
}

namespace bim::game
{
  class contest;
  class contest_result;
  struct animation_state;
  struct fractional_position_on_grid;
  struct invisibility_state;
  struct player;
}

namespace bim::net
{
  class contest_runner;
  struct game_launch_event;
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
  class fog_display;
  class player;
  class player_progress_tracker;

  class online_game
  {
    DECLARE_SIGNAL(void(const bim::game::contest_result&), game_over,
                   m_game_over)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((player_progress_tracker*)(player_progress_tracker))          //
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
    void resize_to_block_width(const std::vector<T*>& nodes) const;
    template <typename T>
    void resize_to_block_width(std::span<T* const> nodes) const;

    void apply_inputs();

    void reset_z_order();
    void refresh_display();
    void display_static_walls();
    void display_falling_blocks();
    void display_brick_walls();
    void display_players();
    void display_player(bool local_still_alive, entt::entity e,
                        const bim::game::player& player,
                        const bim::game::fractional_position_on_grid& p,
                        const bim::game::animation_state& a);
    void display_bombs();
    void display_flames();
    void display_bomb_power_ups();
    void display_flame_power_ups();
    void display_invisibility_power_ups();
    void display_main_timer();

    void display_at(std::size_t arena_y, ax::Node& node,
                    const ax::Vec2& position);

    ax::Vec2 grid_position_to_displayed_block_center(float x, float y) const;
    ax::Vec2 grid_position_to_display(float x, float y) const;

    void stop();

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 8);

    const iscool::style::declaration& m_style_pad_on_the_left;
    const iscool::style::declaration& m_style_pad_on_the_right;
    const iscool::style::declaration& m_style_use_joystick;
    const iscool::style::declaration& m_style_use_d_pad;
    const std::array<const iscool::style::declaration*, 4> m_style_player;

    iscool::signals::scoped_connection m_tick_connection;

    std::chrono::nanoseconds m_last_tick_date;
    std::unique_ptr<bim::game::contest> m_contest;
    std::unique_ptr<iscool::net::message_channel> m_game_channel;
    std::unique_ptr<bim::net::game_update_exchange> m_update_exchange;
    std::unique_ptr<bim::net::contest_runner> m_contest_runner;

    std::unique_ptr<bim::axmol::widget::animation_cache> m_animation_cache;

    std::vector<player*> m_players;
    std::array<std::vector<ax::Sprite*>,
               bim::game::cell_neighborhood_layout_count>
        m_walls;
    std::vector<ax::Sprite*> m_falling_blocks;
    std::vector<ax::Sprite*> m_falling_blocks_shadows;
    std::vector<ax::Sprite*> m_brick_walls;
    std::vector<ax::Sprite*> m_bombs;
    std::vector<ax::Sprite*> m_flames;
    std::vector<ax::Sprite*> m_bomb_power_ups;
    std::vector<ax::Sprite*> m_flame_power_ups;
    std::vector<ax::Sprite*> m_invisibility_power_ups;

    std::unique_ptr<fog_display> m_fog;

    std::vector<int> m_z_order;

    const std::string m_flame_center_asset_name;
    const std::string m_flame_arm_asset_name;
    const std::string m_flame_end_asset_name;

    const float m_arena_width_in_blocks;
    arena_display_config m_display_config;

    std::uint8_t m_local_player_index;

    bool m_bomb_drop_requested;
    bool m_use_stick;

    bim::axmol::widget::named_node_group m_nodes;
  };
}
