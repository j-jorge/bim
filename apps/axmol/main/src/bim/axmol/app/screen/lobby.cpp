// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/lobby.hpp>

#include <bim/axmol/app/analytics_service.hpp>
#include <bim/axmol/app/part/wallet.hpp>
#include <bim/axmol/app/popup/debug_popup.hpp>
#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/popup/player_statistics_popup.hpp>
#include <bim/axmol/app/popup/settings_popup.hpp>
#include <bim/axmol/app/preference/arena_stats.hpp>
#include <bim/axmol/app/shop_support.hpp>

#include <bim/axmol/input/observer/tap_observer.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/factory/progress_timer.hpp>
#include <bim/axmol/widget/merge_named_node_groups.hpp>
#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/axmol/find_child_by_path.hpp>

#include <bim/net/exchange/hello_exchange.hpp>
#include <bim/net/message/hello_ok.hpp>
#include <bim/net/session_handler.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/i18n/numeric.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/system/open_url.hpp>

#include <axmol/2d/Label.h>
#include <axmol/2d/ProgressTimer.h>

#define x_widget_scope bim::axmol::app::lobby::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, settings_button)                       \
      x_widget(bim::axmol::widget::button, play_button)                       \
          x_widget(bim::axmol::widget::button, shop_button)                   \
              x_widget(bim::axmol::widget::button, stats_button)              \
                  x_widget(bim::axmol::widget::button, debug_button)          \
                      x_widget(ax::Node, debug_activator)

#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::lobby, play, m_play);
IMPLEMENT_SIGNAL(bim::axmol::app::lobby, shop, m_shop);
IMPLEMENT_SIGNAL(bim::axmol::app::lobby, reset, m_reset);

bim::axmol::app::lobby::lobby(const context& context,
                              const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_server_statistics_label(
        *dynamic_cast<ax::Label*>(bim::axmol::find_child_by_path(
            *m_controls->play_button,
            *style.get_string("play-button-server-stats-label-path"))))
  , m_hello_exchange(new bim::net::hello_exchange(
        context.get_session_handler()->message_stream()))
  , m_wallet(new wallet(context, *style.get_declaration("wallet")))
  , m_settings(new settings_popup(context, *style.get_declaration("settings")))
  , m_player_statistics(new player_statistics_popup(
        context, *style.get_declaration("player-statistics")))
  , m_message(
        new message_popup(context, *style.get_declaration("message-popup")))
  , m_debug(
        new debug_popup(context, *style.get_declaration("debug"), *m_wallet))
  , m_debug_tap(*m_controls->debug_activator)
  , m_debug_activator_counter(0)
{
  m_all_nodes = m_controls->all_nodes;
  bim::axmol::widget::merge_named_node_groups(m_all_nodes,
                                              m_wallet->display_nodes());

  m_hello_exchange->connect_to_updated(
      [this](const bim::net::hello_ok& message)
      {
        update_server_stats(message);
      });

  if (m_context.get_enable_debug())
    enable_debug();

  m_inputs.push_back(m_wallet->input_node());
  m_wallet->connect_to_clicked(
      [this]()
      {
        open_shop_from_wallet();
      });

  m_inputs.push_back(m_controls->shop_button->input_node());
  m_controls->shop_button->connect_to_clicked(
      [this]()
      {
        open_shop_from_button();
      });

  m_inputs.push_back(m_controls->stats_button->input_node());
  m_controls->stats_button->connect_to_clicked(
      [this]()
      {
        open_player_stats();
      });

  m_debug_tap->connect_to_release(
      [this]() -> void
      {
        increment_debug_activator_counter();
      });
  m_inputs.push_back(std::move(m_debug_tap));

  m_inputs.push_back(m_controls->debug_button->input_node());
  m_controls->debug_button->connect_to_clicked(
      [this]()
      {
        show_debug();
      });

  m_inputs.push_back(m_controls->settings_button->input_node());
  m_controls->settings_button->connect_to_clicked(
      [this]()
      {
        show_settings();
      });
  m_settings->connect_to_reset(
      [this]()
      {
        m_reset();
      });

  m_inputs.push_back(m_controls->play_button->input_node());
  m_controls->play_button->connect_to_clicked(
      [this]()
      {
        play_online();
      });
}

bim::axmol::app::lobby::~lobby() = default;

bim::axmol::input::node_reference bim::axmol::app::lobby::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::lobby::nodes() const
{
  return m_all_nodes;
}

void bim::axmol::app::lobby::attached()
{
  m_wallet->attached();
}

void bim::axmol::app::lobby::displayed()
{}

void bim::axmol::app::lobby::displaying()
{
  m_wallet->enter();
  bim::net::session_handler& session_handler =
      *m_context.get_session_handler();

  m_session_connection = session_handler.connect_to_connected(
      [this]()
      {
        apply_connected_state();
      });
  apply_connected_state();
}

void bim::axmol::app::lobby::closing()
{
  m_hello_exchange->stop();
  m_session_connection.disconnect();
  m_message_connection.disconnect();
}

void bim::axmol::app::lobby::update_server_stats(
    const bim::net::hello_ok& message)
{
  if (message.games_now >= 10)
    {
      m_server_statistics_label.setString(fmt::format(
          fmt::runtime(ic_ngettext("{} game being played right now.",
                                   "{} games being played right now.",
                                   message.games_now)),
          iscool::i18n::numeric::to_string(message.games_now)));
      return;
    }

  if (message.sessions_now >= 10)
    {
      m_server_statistics_label.setString(fmt::format(
          fmt::runtime(ic_ngettext("{} player connected right now.",
                                   "{} players connected right now.",
                                   message.sessions_now)),
          iscool::i18n::numeric::to_string(message.sessions_now)));
      return;
    }

  if (message.games_last_hour >= 10)
    {
      m_server_statistics_label.setString(fmt::format(
          fmt::runtime(ic_ngettext("{} game played in the last hour.",
                                   "{} games played in the last hour.",
                                   message.games_last_hour)),
          iscool::i18n::numeric::to_string(message.games_last_hour)));
      return;
    }

  if (message.sessions_last_hour >= 10)
    {
      m_server_statistics_label.setString(fmt::format(
          fmt::runtime(ic_ngettext("{} player connected in the last hour.",
                                   "{} players connected in the last hour.",
                                   message.sessions_last_hour)),
          iscool::i18n::numeric::to_string(message.sessions_last_hour)));
      return;
    }

  if (message.games_last_day >= 10)
    {
      m_server_statistics_label.setString(fmt::format(
          fmt::runtime(ic_ngettext("{} game played in the last 24 hours.",
                                   "{} games played in the last 24 hours.",
                                   message.games_last_day)),
          iscool::i18n::numeric::to_string(message.games_last_day)));
      return;
    }

  if (message.sessions_last_day >= 10)
    {
      m_server_statistics_label.setString(fmt::format(
          fmt::runtime(
              ic_ngettext("{} player connected in the last 24 hours.",
                          "{} players connected in the last 24 hours.",
                          message.sessions_last_day)),
          iscool::i18n::numeric::to_string(message.sessions_last_day)));
      return;
    }

  if (message.games_last_month >= 10)
    {
      m_server_statistics_label.setString(fmt::format(
          fmt::runtime(ic_ngettext("{} game played in the last 30 days.",
                                   "{} games played in the last 30 days.",
                                   message.games_last_month)),
          iscool::i18n::numeric::to_string(message.games_last_month)));
      return;
    }

  if (message.sessions_last_month >= 10)
    {
      m_server_statistics_label.setString(fmt::format(
          fmt::runtime(ic_ngettext("{} player connected in the last 30 days.",
                                   "{} players connected in the last 30 days.",
                                   message.sessions_last_month)),
          iscool::i18n::numeric::to_string(message.sessions_last_month)));
      return;
    }

  m_server_statistics_label.setString(
      fmt::format(fmt::runtime(ic_gettext("Connected to {}.")), message.name));
}

void bim::axmol::app::lobby::apply_connected_state()
{
  const bool connected = m_context.get_session_handler()->connected();

  if (connected)
    m_hello_exchange->start();

  m_controls->play_button->enable(connected);
}

void bim::axmol::app::lobby::increment_debug_activator_counter()
{
  if (m_debug_activator_counter >= 24)
    return;

  ++m_debug_activator_counter;

  if (m_debug_activator_counter == 24)
    enable_debug();
}

void bim::axmol::app::lobby::enable_debug()
{
  m_controls->debug_button->setVisible(true);
  m_controls->debug_activator->setVisible(false);
}

void bim::axmol::app::lobby::show_debug()
{
  m_context.get_analytics()->event(
      "button", { { "id", "debug" }, { "where", "lobby" } });
  m_debug->show();
}

void bim::axmol::app::lobby::show_settings()
{
  m_context.get_analytics()->event(
      "button", { { "id", "settings" }, { "where", "lobby" } });
  m_settings->show();
}

void bim::axmol::app::lobby::play_online()
{
  m_context.get_analytics()->event("button",
                                   { { "id", "play" }, { "where", "lobby" } });
  m_play();
}

void bim::axmol::app::lobby::open_shop_from_wallet()
{
  m_context.get_analytics()->event(
      "button", { { "id", "wallet" }, { "where", "lobby" } });
  open_shop();
}

void bim::axmol::app::lobby::open_shop_from_button()
{
  m_context.get_analytics()->event("button",
                                   { { "id", "shop" }, { "where", "lobby" } });
  open_shop();
}

void bim::axmol::app::lobby::open_shop()
{
  if (is_shop_supported())
    m_shop();
#if BIM_PURE_FOSS
  #define gettext_foss_only(s) ic_gettext(s)
  else
    {
      m_message_connection = m_message->connect_to_ok(
          []()
          {
            iscool::system::open_url("https://github.com/sponsors/j-jorge");
          });

      m_message->show_yes_no(gettext_foss_only(
          "The shop is not available on this platform, yet you can support "
          "the developers with donations! Should I open the donations page?"));
    }
  #undef ignore_when_non_foss
#endif
}

void bim::axmol::app::lobby::open_player_stats() const
{
  m_context.get_analytics()->event(
      "button", { { "id", "player-stats" }, { "where", "lobby" } });
  m_player_statistics->show();
}
