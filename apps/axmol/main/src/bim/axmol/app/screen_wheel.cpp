// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen_wheel.hpp>

#include <bim/axmol/app/application_event_dispatcher.hpp>
#include <bim/axmol/app/main_scene.hpp>
#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/screen/end_game.hpp>
#include <bim/axmol/app/screen/game_features.hpp>
#include <bim/axmol/app/screen/lobby.hpp>
#include <bim/axmol/app/screen/matchmaking.hpp>
#include <bim/axmol/app/screen/online_game.hpp>
#include <bim/axmol/app/screen/shop.hpp>
#include <bim/axmol/app/shop_intent.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/factory/clipping_rectangle_node.hpp>

#include <bim/app/analytics/error.hpp>
#include <bim/app/analytics_service.hpp>
#include <bim/app/player_progress_tracker.hpp>
#include <bim/app/preference/wallet.hpp>
#include <bim/app/shop_support.hpp>

#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/exchange/keep_alive_exchange.hpp>
#include <bim/net/session_handler.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/system/open_url.hpp>

#include <axmol/2d/ClippingRectangleNode.h>

#define x_widget_scope bim::axmol::app::screen_wheel::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::ClippingRectangleNode, lobby)                                  \
      x_widget(ax::ClippingRectangleNode, matchmaking)                        \
          x_widget(ax::ClippingRectangleNode, online_game)                    \
              x_widget(ax::ClippingRectangleNode, end_game)                   \
                  x_widget(ax::ClippingRectangleNode, game_features)          \
                      x_widget(ax::ClippingRectangleNode, shop)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/system/keep_screen_on.hpp>

#include <axmol/2d/ActionEase.h>
#include <axmol/2d/ActionInstant.h>
#include <axmol/2d/ActionInterval.h>
#include <axmol/2d/Node.h>

IMPLEMENT_SIGNAL(bim::axmol::app::screen_wheel, reset, m_reset)

bim::axmol::app::screen_wheel::screen_wheel(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_main_container(ax::Node::create())
  , m_controls(*context.get_widget_context(),
               *style.get_declaration("widgets"))
  , m_player_progress_tracker(new bim::app::player_progress_tracker(
        *context.get_analytics(), *context.get_local_preferences()))
  , m_lobby(new lobby(context, *style.get_declaration("lobby")))
  , m_matchmaking(
        new matchmaking(context, *style.get_declaration("matchmaking")))
  , m_end_game(new end_game(context, *style.get_declaration("end-game")))
  , m_game_features(
        new game_features(context, *style.get_declaration("game-features")))
  , m_shop(new shop(context, *style.get_declaration("shop")))
  , m_message_popup(
        new message_popup(context, *style.get_declaration("message-popup")))
  , m_yes_no_popup(
        new message_popup(context, *style.get_declaration("yes-no-popup")))
  , m_keep_alive(new bim::net::keep_alive_exchange(
        context.get_session_handler()->message_stream()))
  , m_leave_shop(nullptr)
{
  m_main_container->setName("screen-wheel");

  m_context.set_player_progress_tracker(m_player_progress_tracker.get());

  m_online_game.reset(
      new online_game(m_context, *style.get_declaration("online-game")));

  m_context.get_main_scene()->add_in_main_canvas(*m_main_container,
                                                 m_inputs.root());

  // Add and arrange the screen containers into the main container.
  map_nodes(*m_main_container, m_controls->all_nodes, style, "bounds");

  // Put the screens in their containers and apply their layout such that the
  // nodes are set up to their basic position and size. From then on we will
  // just handle their container.
  map_nodes(*m_controls->lobby, m_lobby->nodes(), style, "lobby-bounds");
  m_lobby->attached();

  map_nodes(*m_controls->matchmaking, m_matchmaking->display_nodes(), style,
            "matchmaking-bounds");
  m_matchmaking->attached();
  m_controls->matchmaking->removeFromParent();

  map_nodes(*m_controls->online_game, m_online_game->nodes(), style,
            "online-game-bounds");
  m_online_game->attached();
  m_controls->online_game->removeFromParent();

  map_nodes(*m_controls->end_game, m_end_game->display_nodes(), style,
            "end-game-bounds");
  m_controls->end_game->removeFromParent();

  map_nodes(*m_controls->game_features, m_game_features->display_nodes(),
            style, "game-features-bounds");
  m_game_features->attached();
  m_controls->game_features->removeFromParent();

  map_nodes(*m_controls->shop, m_shop->display_nodes(), style, "shop-bounds");
  m_shop->attached();
  m_controls->shop->removeFromParent();

  wire_permanent_connections();

  m_message_popup->connect_to_ok(
      [this]() -> void
      {
        m_reset();
      });

  m_lobby->connect_to_reset(
      [this]() -> void
      {
        m_reset();
      });

  m_session_handler_connection =
      m_context.get_session_handler()->connect_to_connected(
          [this]()
          {
            connect_keep_alive();
          });

  connect_keep_alive();

  configure_screen_transitions();

  const ax::Rect clipping_region(ax::Vec2(0, 0),
                                 m_main_container->getContentSize());
  m_controls->lobby->setClippingRegion(clipping_region);
  m_controls->shop->setClippingRegion(clipping_region);
  m_controls->matchmaking->setClippingRegion(clipping_region);
  m_controls->online_game->setClippingRegion(clipping_region);
  m_controls->end_game->setClippingRegion(clipping_region);
  m_controls->game_features->setClippingRegion(clipping_region);

  // Start on the lobby, In the initial state
  m_active_view = m_controls->lobby;
  m_lobby->displaying();
  lobby_displayed();
}

bim::axmol::app::screen_wheel::~screen_wheel()
{
  m_context.get_main_scene()->remove_from_main_canvas(*m_main_container);
}

void bim::axmol::app::screen_wheel::map_nodes(
    ax::Node& container, const bim::axmol::widget::named_node_group& nodes,
    const iscool::style::declaration& style,
    const std::string& bounds_style_name) const
{
  bim::axmol::widget::add_group_as_children(container, nodes);
  bim::axmol::widget::apply_bounds(*m_context.get_widget_context(), nodes,
                                   *style.get_declaration(bounds_style_name));
}

void bim::axmol::app::screen_wheel::wire_permanent_connections()
{
  m_lobby->connect_to_play(
      [this]()
      {
        animate_lobby_to_matchmaking();
      });

  m_lobby->connect_to_game_features(
      [this]()
      {
        animate_lobby_to_game_features();
      });

  m_matchmaking->connect_to_start_game(
      [this](const bim::net::game_launch_event& event)
      {
        animate_matchmaking_to_game(event);
      });

  m_matchmaking->connect_to_back(
      [this]()
      {
        animate_matchmaking_to_lobby();
      });

  m_online_game->connect_to_game_over(
      [this](const bim::net::contest_result& result)
      {
        animate_game_to_end_game(result);
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

  m_shop->connect_to_back(
      [this]()
      {
        assert(m_leave_shop != nullptr);
        (this->*m_leave_shop)();
      });

  m_lobby->connect_to_shop(
      [this](shop_intent intent)
      {
        animate_lobby_to_shop(intent);
      });

  m_game_features->connect_to_back(
      [this]()
      {
        animate_game_features_to_lobby();
      });

  m_game_features->connect_to_shop(
      [this](shop_intent intent)
      {
        animate_game_features_to_shop(intent);
      });
}

void bim::axmol::app::screen_wheel::connect_keep_alive()
{
  bim::net::session_handler& session_handler =
      *m_context.get_session_handler();

  if (!session_handler.connected())
    return;

  m_silently_reconnect = true;
  m_keep_alive_connection = m_keep_alive->connect_to_disconnected(
      [this]() -> void
      {
        disconnected();
      });
  m_keep_alive->start(session_handler.session_id());
}

void bim::axmol::app::screen_wheel::configure_screen_transitions()
{
  const ax::Node* const lobby = m_controls->lobby;
  const ax::Node* const game_features = m_controls->game_features;
  const ax::Node* const shop = m_controls->shop;
  const ax::Node* const matchmaking = m_controls->matchmaking;
  const ax::Node* const online_game = m_controls->online_game;
  const ax::Node* const end_game = m_controls->end_game;

  m_screen_index[lobby] = m_screen_index.size();
  m_screen_index[shop] = m_screen_index.size();
  m_screen_index[game_features] = m_screen_index.size();
  m_screen_index[matchmaking] = m_screen_index.size();
  m_screen_index[online_game] = m_screen_index.size();
  m_screen_index[end_game] = m_screen_index.size();

  m_displayed[m_screen_index[lobby]] = &screen_wheel::lobby_displayed;
  m_displayed[m_screen_index[shop]] = &screen_wheel::shop_displayed;
  m_displayed[m_screen_index[game_features]] =
      &screen_wheel::game_features_displayed;
  m_displayed[m_screen_index[matchmaking]] =
      &screen_wheel::matchmaking_displayed;
  m_displayed[m_screen_index[online_game]] =
      &screen_wheel::online_game_displayed;
  m_displayed[m_screen_index[end_game]] = &screen_wheel::end_game_displayed;

  m_screen_to_screen_direction = bim::table_2d<ax::Vec2>(
      m_screen_index.size(), m_screen_index.size(), ax::Vec2(1, 0));

  const auto set_screen_direction =
      [this](const ax::Node* from, const ax::Node* to, float dx, float dy)
  {
    m_screen_to_screen_direction(m_screen_index[from], m_screen_index[to]) =
        ax::Vec2(dx, dy);
    m_screen_to_screen_direction(m_screen_index[to], m_screen_index[from]) =
        ax::Vec2(-dx, -dy);
  };

  set_screen_direction(lobby, shop, -1, 0);
  set_screen_direction(lobby, game_features, -1, 0);
  set_screen_direction(lobby, matchmaking, 1, 0);
  set_screen_direction(game_features, shop, -1, 0);
  set_screen_direction(matchmaking, online_game, 0, 1);
  set_screen_direction(online_game, lobby, 0, -1);
  set_screen_direction(online_game, end_game, 0, -1);
  set_screen_direction(end_game, matchmaking, -1, 0);
  set_screen_direction(end_game, lobby, -1, 0);
}

void bim::axmol::app::screen_wheel::switch_view(ax::Node& new_view)
{
  assert(&new_view != m_active_view);

  m_main_container->addChild(&new_view);

  const ax::Vec2& direction_to_initial_position = m_screen_to_screen_direction(
      m_screen_index[m_active_view], m_screen_index[&new_view]);

  const ax::Vec2 size = m_main_container->getContentSize();
  assert(new_view.getContentSize() == size);

  const ax::Vec2 half_size = size / 2;
  const ax::Vec2 start_middle =
      half_size + direction_to_initial_position * size;
  const ax::Vec2 start_position =
      start_middle - (half_size - size * new_view.getAnchorPoint());
  const ax::Vec2 enter_translation = -direction_to_initial_position * size;
  const float animation_duration = 0.5;

  new_view.setPosition(start_position);

  new_view.runAction(ax::EaseCircleActionOut::create(
      ax::MoveBy::create(animation_duration, enter_translation)));

  const auto remove_old_view = [this, &new_view]() -> void
  {
    m_active_view->removeFromParent();
    m_active_view = &new_view;
    (this->*m_displayed[m_screen_index[&new_view]])();
  };

  m_active_view->runAction(
      ax::Sequence::create(ax::EaseCircleActionOut::create(ax::MoveBy::create(
                               animation_duration, enter_translation)),
                           ax::CallFunc::create(remove_old_view), nullptr));
}

void bim::axmol::app::screen_wheel::animate_lobby_to_matchmaking()
{
  m_inputs.erase(m_lobby->input_node());

  m_lobby->closing();
  display_matchmaking();
}

void bim::axmol::app::screen_wheel::animate_matchmaking_to_game(
    const bim::net::game_launch_event& event)
{
  m_inputs.erase(m_matchmaking->input_node());
  m_matchmaking->closing();
  display_online_game(event);
}

void bim::axmol::app::screen_wheel::animate_matchmaking_to_lobby()
{
  iscool::system::keep_screen_on(false);

  m_inputs.erase(m_matchmaking->input_node());
  m_matchmaking->closing();
  display_lobby();
}

void bim::axmol::app::screen_wheel::animate_game_to_end_game(
    const bim::net::contest_result& result)
{
  m_inputs.erase(m_online_game->input_node());
  m_online_game->closing();
  display_end_game(result);
}

void bim::axmol::app::screen_wheel::animate_end_game_to_lobby()
{
  m_inputs.erase(m_end_game->input_node());
  m_end_game->closing();
  display_lobby();
}

void bim::axmol::app::screen_wheel::animate_end_game_to_matchmaking()
{
  m_inputs.erase(m_end_game->input_node());
  m_end_game->closing();
  display_matchmaking();
}

void bim::axmol::app::screen_wheel::animate_lobby_to_shop(shop_intent intent)
{
  if (!can_open_shop(intent))
    return;

  m_leave_shop = &screen_wheel::animate_shop_to_lobby;
  m_inputs.erase(m_lobby->input_node());

  m_lobby->closing();
  display_shop();
}

void bim::axmol::app::screen_wheel::animate_shop_to_lobby()
{
  m_inputs.erase(m_shop->input_node());

  display_lobby();
}

void bim::axmol::app::screen_wheel::animate_shop_to_game_features()
{
  m_inputs.erase(m_shop->input_node());

  display_game_features();
}

void bim::axmol::app::screen_wheel::animate_lobby_to_game_features()
{
  m_leave_shop = &screen_wheel::animate_shop_to_lobby;
  m_inputs.erase(m_lobby->input_node());

  m_lobby->closing();
  display_game_features();
}

void bim::axmol::app::screen_wheel::animate_game_features_to_lobby()
{
  m_inputs.erase(m_game_features->input_node());

  m_game_features->closing();
  display_lobby();
}

void bim::axmol::app::screen_wheel::animate_game_features_to_shop(
    shop_intent intent)
{
  if (!can_open_shop(intent))
    return;

  m_leave_shop = &screen_wheel::animate_shop_to_game_features;
  m_inputs.erase(m_game_features->input_node());

  m_game_features->closing();
  display_shop();
}

void bim::axmol::app::screen_wheel::display_lobby()
{
  m_context.get_analytics()->screen(
      "lobby", { { "coins", std::to_string(bim::app::coins_balance(
                                *m_context.get_local_preferences())) } });

  m_lobby->displaying();
  switch_view(*m_controls->lobby);
}

void bim::axmol::app::screen_wheel::lobby_displayed()
{
  m_inputs.push_back(m_lobby->input_node());
  m_lobby->displayed();
  m_context.get_event_dispatcher()->dispatch("lobby-ready");
}

void bim::axmol::app::screen_wheel::display_matchmaking()
{
  m_context.get_analytics()->screen("matchmaking");

  m_matchmaking->displaying();
  switch_view(*m_controls->matchmaking);
}

void bim::axmol::app::screen_wheel::matchmaking_displayed()
{
  iscool::system::keep_screen_on(true);
  m_inputs.push_back(m_matchmaking->input_node());
  m_matchmaking->displayed();
  m_context.get_event_dispatcher()->dispatch("matchmaking-ready");
}

void bim::axmol::app::screen_wheel::display_online_game(
    const bim::net::game_launch_event& event)
{
  m_context.get_analytics()->screen(
      "online-game",
      { { "player-count", std::to_string(event.fingerprint.player_count) } });

  m_online_game->displaying(event);
  m_end_game->game_started(event);
  switch_view(*m_controls->online_game);
}

void bim::axmol::app::screen_wheel::online_game_displayed()
{
  m_inputs.push_back(m_online_game->input_node());
  m_online_game->displayed();
  m_context.get_event_dispatcher()->dispatch("online-game-ready");
}

void bim::axmol::app::screen_wheel::display_end_game(
    const bim::net::contest_result& result)
{
  m_context.get_analytics()->screen(
      "end-game", { { "coins", std::to_string(bim::app::coins_balance(
                                   *m_context.get_local_preferences())) } });

  m_end_game->displaying(result);
  switch_view(*m_controls->end_game);
}

void bim::axmol::app::screen_wheel::end_game_displayed()
{
  iscool::system::keep_screen_on(false);
  m_inputs.push_back(m_end_game->input_node());
  m_end_game->displayed();
  m_context.get_event_dispatcher()->dispatch("end-game-ready");
}

void bim::axmol::app::screen_wheel::display_shop()
{
  m_shop->displaying();
  switch_view(*m_controls->shop);
}

void bim::axmol::app::screen_wheel::shop_displayed()
{
  m_context.get_analytics()->screen("shop");

  m_inputs.push_back(m_shop->input_node());
  m_shop->displayed();
  m_context.get_event_dispatcher()->dispatch("shop-ready");
}

void bim::axmol::app::screen_wheel::display_game_features()
{
  m_game_features->displaying();
  switch_view(*m_controls->game_features);
}

void bim::axmol::app::screen_wheel::game_features_displayed()
{
  m_context.get_analytics()->screen("game-features");

  m_inputs.push_back(m_game_features->input_node());
  m_context.get_event_dispatcher()->dispatch("game-features-ready");
}

void bim::axmol::app::screen_wheel::disconnected()
{
  m_keep_alive->stop();

  if ((m_active_view == m_controls->online_game) || !m_silently_reconnect)
    {
      bim::app::error(*m_context.get_analytics(), "disconnected");

      m_online_game->closing();

      m_message_popup->show(ic_gettext("You have been disconnected :("));
    }
  else
    {
      ic_log(iscool::log::nature::info(), "screen_wheel",
             "Disconnected, reconnecting.");
      m_silently_reconnect = false;
      m_context.get_session_handler()->reconnect();
    }
}

bool bim::axmol::app::screen_wheel::can_open_shop(shop_intent intent)
{
  if (bim::app::is_shop_supported())
    return true;

  if (intent == shop_intent::user_request)
    {
      m_message_connection = m_yes_no_popup->connect_to_ok(
          []()
          {
            iscool::system::open_url("https://github.com/sponsors/j-jorge");
          });

      m_yes_no_popup->show_yes_no(ic_gettext(
          "The shop is not available on this platform, yet you can support "
          "the developers with donations! Should I open the donations page?"));
    }

  return false;
}
