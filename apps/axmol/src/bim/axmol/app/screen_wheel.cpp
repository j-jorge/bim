// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen_wheel.hpp>

#include <bim/axmol/app/main_scene.hpp>
#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/screen/end_game.hpp>
#include <bim/axmol/app/screen/lobby.hpp>
#include <bim/axmol/app/screen/matchmaking.hpp>
#include <bim/axmol/app/screen/online_game.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/context.hpp>

#include <bim/net/exchange/keep_alive_exchange.hpp>
#include <bim/net/session_handler.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/signals/implement_signal.hpp>

#define x_widget_scope bim::axmol::app::screen_wheel::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Node, lobby) x_widget(ax::Node, matchmaking)                   \
      x_widget(ax::Node, online_game) x_widget(ax::Node, end_game)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/system/keep_screen_on.hpp>

#include <axmol/2d/Node.h>
#include <axmol/base/Director.h>

static constexpr int g_fps_in_menus = 30;
static constexpr int g_fps_in_game = 60;

IMPLEMENT_SIGNAL(bim::axmol::app::screen_wheel, reset, m_reset)

bim::axmol::app::screen_wheel::screen_wheel(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_main_container(ax::Node::create())
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_lobby(new lobby(m_context, *style.get_declaration("lobby")))
  , m_matchmaking(
        new matchmaking(m_context, *style.get_declaration("matchmaking")))
  , m_online_game(
        new online_game(m_context, *style.get_declaration("online-game")))
  , m_end_game(new end_game(m_context, *style.get_declaration("end-game")))
  , m_message_popup(
        new message_popup(m_context, *style.get_declaration("message-popup")))
  , m_keep_alive(new bim::net::keep_alive_exchange(
        m_context.get_session_handler()->message_stream()))
{
  ax::Director::getInstance()->setAnimationInterval(1.f / g_fps_in_menus);

  m_context.get_main_scene()->add_in_main_canvas(*m_main_container,
                                                 m_inputs.root());

  // Add and arrange the screen containers into the main container.
  map_nodes(*m_main_container, m_controls->all_nodes, style, "bounds");

  // Put the screens in their containers and apply their layout such that the
  // nodes are set up to their basic position and size. From then on we will
  // just handle their container.
  map_nodes(*m_controls->lobby, m_lobby->nodes(), style, "lobby-bounds");

  map_nodes(*m_controls->matchmaking, m_matchmaking->nodes(), style,
            "matchmaking-bounds");
  m_controls->matchmaking->removeFromParent();

  map_nodes(*m_controls->online_game, m_online_game->nodes(), style,
            "online-game-bounds");
  m_online_game->attached();
  m_controls->online_game->removeFromParent();

  map_nodes(*m_controls->end_game, m_end_game->nodes(), style,
            "end-game-bounds");
  m_controls->end_game->removeFromParent();

  wire_permanent_connections();

  m_message_popup->connect_to_ok(
      [this]() -> void
      {
        m_reset();
      });

  m_context.get_session_handler()->connect_to_connected(
      [this]()
      {
        connect_keep_alive();
      });

  connect_keep_alive();

  // Start on the lobby, In the initial state
  m_active_view = m_controls->lobby;
  lobby_displayed();
}

bim::axmol::app::screen_wheel::~screen_wheel() = default;

void bim::axmol::app::screen_wheel::map_nodes(
    ax::Node& container, const bim::axmol::widget::named_node_group& nodes,
    const iscool::style::declaration& style,
    const std::string& bounds_style_name) const
{
  bim::axmol::widget::add_group_as_children(container, nodes);
  bim::axmol::widget::apply_bounds(m_context.get_widget_context(), nodes,
                                   *style.get_declaration(bounds_style_name));
}

void bim::axmol::app::screen_wheel::wire_permanent_connections()
{
  m_lobby->connect_to_play(
      [this]()
      {
        animate_lobby_to_matchmaking();
      });

  m_matchmaking->connect_to_start_game(
      [this](const bim::net::game_launch_event& event)
      {
        animate_matchmaking_to_game(event);
      });

  m_online_game->connect_to_game_over(
      [this](const bim::game::contest_result& result)
      {
        animate_game_to_end_game(result);
      });

  m_online_game->connect_to_disconnected(
      [this]()
      {
        disconnected();
      });

  m_end_game->connect_to_quit(
      [this]()
      {
        animate_end_game_to_lobby();
      });

  m_end_game->connect_to_revenge(
      [this]()
      {
        animate_end_game_to_matchmaking();
      });

  m_keep_alive->connect_to_disconnected(
      [this]()
      {
        disconnected();
      });
}

void bim::axmol::app::screen_wheel::connect_keep_alive()
{
  bim::net::session_handler& session_handler =
      *m_context.get_session_handler();

  if (!session_handler.connected())
    return;

  m_keep_alive_connection = m_keep_alive->connect_to_disconnected(
      [this]() -> void
      {
        disconnected();
      });
  m_keep_alive->start(session_handler.session_id());
}

void bim::axmol::app::screen_wheel::switch_view(ax::Node& new_view)
{
  m_active_view->removeFromParent();
  m_active_view = &new_view;
  m_main_container->addChild(m_active_view);
}

void bim::axmol::app::screen_wheel::animate_lobby_to_matchmaking()
{
  m_inputs.erase(m_lobby->input_node());

  m_lobby->closing();
  switch_view(*m_controls->matchmaking);
  m_matchmaking->displaying();

  matchmaking_displayed();
}

void bim::axmol::app::screen_wheel::animate_matchmaking_to_game(
    const bim::net::game_launch_event& event)
{
  iscool::system::keep_screen_on(false);

  m_inputs.erase(m_matchmaking->input_node());
  m_matchmaking->closing();

  switch_view(*m_controls->online_game);
  m_online_game->displaying(event);
  m_end_game->game_started(event);

  online_game_displayed();
}

void bim::axmol::app::screen_wheel::animate_game_to_end_game(
    const bim::game::contest_result& result)
{
  ax::Director::getInstance()->setAnimationInterval(1.f / g_fps_in_menus);

  m_inputs.erase(m_online_game->input_node());
  m_online_game->closing();

  switch_view(*m_controls->end_game);
  m_end_game->displaying(result);

  end_game_displayed();
}

void bim::axmol::app::screen_wheel::animate_end_game_to_lobby()
{
  m_inputs.erase(m_end_game->input_node());
  m_end_game->closing();

  switch_view(*m_controls->lobby);

  lobby_displayed();
}

void bim::axmol::app::screen_wheel::animate_end_game_to_matchmaking()
{
  m_inputs.erase(m_end_game->input_node());
  m_end_game->closing();

  switch_view(*m_controls->matchmaking);
  m_matchmaking->displaying();

  matchmaking_displayed();
}

void bim::axmol::app::screen_wheel::lobby_displayed()
{
  m_inputs.push_back(m_lobby->input_node());
  m_lobby->displayed();
}

void bim::axmol::app::screen_wheel::matchmaking_displayed()
{
  iscool::system::keep_screen_on(true);
  m_inputs.push_back(m_matchmaking->input_node());
  m_matchmaking->displayed();
}

void bim::axmol::app::screen_wheel::online_game_displayed()
{
  ax::Director::getInstance()->setAnimationInterval(1.f / g_fps_in_game);
  m_inputs.push_back(m_online_game->input_node());
  m_online_game->displayed();
}

void bim::axmol::app::screen_wheel::end_game_displayed()
{
  m_inputs.push_back(m_end_game->input_node());
  m_end_game->displayed();
}

void bim::axmol::app::screen_wheel::disconnected()
{
  m_online_game->closing();
  m_keep_alive->stop();

  m_message_popup->show(ic_gettext("You have been disconnected."));
}
