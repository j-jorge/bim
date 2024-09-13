// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <axmol/base/Types.h>

#include <entt/entity/fwd.hpp>

#include <boost/unordered/unordered_map.hpp>

#include <chrono>
#include <thread>
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

namespace iscool::system
{
  class haptic_feedback;
}

namespace ax
{
  class Sprite;
}

namespace iscool::net
{
  class message_channel;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class online_game
  {
    DECLARE_SIGNAL(void(const bim::game::contest_result&), game_over,
                   m_game_over)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context&)(widget_context)) //
            ((bim::net::session_handler*)(session_handler))        //
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
    enum class game_update_thread_state;
    struct shared_game_update;
    struct game_update_thread;

  private:
    void schedule_tick();
    void tick();

    template <typename T>
    void alloc_assets(std::vector<T*>& out,
                      const bim::axmol::widget::context& context,
                      std::size_t count,
                      const iscool::style::declaration& style) const;

    void apply_inputs();

    void refresh_display() const;
    void display_brick_walls() const;
    void display_players() const;
    void display_bombs() const;
    void display_flames() const;
    void display_bomb_power_ups() const;
    void display_flame_power_ups() const;

    ax::Vec2 grid_position_to_displayed_block_center(std::uint8_t x,
                                                     std::uint8_t y) const;
    ax::Vec2 grid_position_to_display(float x, float y) const;

    void stop();

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 5);

    iscool::signals::scoped_connection m_tick_connection;

    std::unique_ptr<bim::game::contest> m_contest;
    std::unique_ptr<iscool::net::message_channel> m_game_channel;
    std::unique_ptr<bim::net::game_update_exchange> m_update_exchange;
    std::unique_ptr<bim::net::contest_runner> m_contest_runner;

    std::unique_ptr<shared_game_update> m_shared_game_update;
    std::thread m_game_update;

    std::vector<ax::Sprite*> m_players;
    std::vector<ax::Sprite*> m_walls;
    std::vector<ax::Sprite*> m_brick_walls;
    std::vector<ax::Sprite*> m_bombs;
    std::vector<ax::Sprite*> m_flames;
    std::vector<ax::Sprite*> m_bomb_power_ups;
    std::vector<ax::Sprite*> m_flame_power_ups;

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
  };
}
