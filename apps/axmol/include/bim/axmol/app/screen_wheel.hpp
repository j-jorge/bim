#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>

#include <memory>
#include <string>

namespace bim
{
  namespace axmol::widget
  {
    class context;
  }

  namespace net
  {
    class session_handler;
    struct game_launch_event;
  }

  namespace game
  {
    class contest_result;
  }
}

namespace iscool
{
  namespace preferences
  {
    class local_preferences;
  }
  namespace style
  {
    class declaration;
  }
}

namespace bim::axmol::app
{
  class end_game;
  class lobby;
  class main_scene;
  class matchmaking;
  class online_game;
  class scene_lock;

  class screen_wheel
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context&)(widget_context)) //
            ((main_scene*)(main_scene))                            //
            ((bim::net::session_handler*)(session_handler))),
        ic_context_no_properties);

  public:
    screen_wheel(const context& context,
                 const iscool::style::declaration& style);
    ~screen_wheel();

  private:
    void map_nodes(ax::Node& container,
                   const bim::axmol::widget::named_node_group& nodes,
                   const iscool::style::declaration& style,
                   const std::string& bounds_style_name) const;

    void wire_permanent_connections();

    void switch_view(ax::Node& new_view);

    void animate_lobby_to_matchmaking();
    void animate_matchmaking_to_game(const bim::net::game_launch_event& event);
    void animate_game_to_end_game(const bim::game::contest_result& result);
    void animate_end_game_to_lobby();

    void lobby_displayed();
    void matchmaking_displayed();
    void online_game_displayed();
    void end_game_displayed();

  private:
    bim::axmol::ref_ptr<ax::Node> m_main_container;
    bim::axmol::input::tree m_inputs;
    ax::Node* m_active_view;

    bim_declare_controls_struct(controls, m_controls, 4);

    std::unique_ptr<lobby> m_lobby;
    std::unique_ptr<matchmaking> m_matchmaking;
    std::unique_ptr<online_game> m_online_game;
    std::unique_ptr<end_game> m_end_game;
  };
}
