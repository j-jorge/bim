// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/lobby.hpp>

#include <bim/axmol/app/part/wallet.hpp>
#include <bim/axmol/app/popup/debug_popup.hpp>
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

#include <bim/net/session_handler.hpp>

#include <iscool/i18n/numeric.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Label.h>
#include <axmol/2d/ProgressTimer.h>

#define x_widget_scope bim::axmol::app::lobby::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, settings_button)                       \
      x_widget(bim::axmol::widget::button, play_button)                       \
          x_widget(bim::axmol::widget::button, debug_button)                  \
              x_widget(ax::Node, debug_activator)                             \
                  x_widget(ax::Label, arena_games_total)                      \
                      x_widget(ax::Label, arena_victories)                    \
                          x_widget(ax::Label, arena_defeats)                  \
                              x_widget(ax::Label, arena_draws)                \
                                  x_widget(ax::Label, success_rate_percents)  \
                                      x_widget(ax::ProgressTimer,             \
                                               success_rate_progress)

#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::lobby, play, m_play);
IMPLEMENT_SIGNAL(bim::axmol::app::lobby, shop, m_shop);

bim::axmol::app::lobby::lobby(const context& context,
                              const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_wallet(new wallet(context, *style.get_declaration("wallet")))
  , m_settings(new settings_popup(context, *style.get_declaration("settings")))
  , m_debug(
        new debug_popup(context, *style.get_declaration("debug"), *m_wallet))
  , m_debug_tap(*m_controls->debug_activator)
  , m_debug_activator_counter(0)
{
  m_all_nodes = m_controls->all_nodes;
  bim::axmol::widget::merge_named_node_groups(m_all_nodes,
                                              m_wallet->display_nodes());

  if (m_context.get_enable_debug())
    enable_debug();

  if (is_shop_supported())
    {
      m_inputs.push_back(m_wallet->input_node());
      m_wallet->connect_to_clicked(
          [this]()
          {
            m_shop();
          });
    }

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
        m_debug->show();
      });

  m_inputs.push_back(m_controls->settings_button->input_node());
  m_controls->settings_button->connect_to_clicked(
      [this]()
      {
        m_settings->show();
      });

  m_inputs.push_back(m_controls->play_button->input_node());
  m_controls->play_button->connect_to_clicked(
      [this]()
      {
        m_play();
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

void bim::axmol::app::lobby::displaying()
{
  const iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  const std::int64_t total_games =
      bim::axmol::app::games_in_arena(preferences);
  const std::int64_t games_win =
      bim::axmol::app::victories_in_arena(preferences);
  const std::int64_t games_defeat =
      bim::axmol::app::defeats_in_arena(preferences);
  const std::int64_t games_draw = total_games - games_win - games_defeat;

  m_controls->arena_games_total->setString(
      iscool::i18n::numeric::to_string(total_games));

  m_controls->arena_victories->setString(
      iscool::i18n::numeric::to_string(games_win));
  m_controls->arena_defeats->setString(
      iscool::i18n::numeric::to_string(games_defeat));
  m_controls->arena_draws->setString(
      iscool::i18n::numeric::to_string(games_draw));

  if (total_games == 0)
    {
      m_controls->success_rate_percents->setString("-");
      m_controls->success_rate_progress->setPercentage(100);
    }
  else
    {
      const float percents = float(games_win * 100) / total_games;

      m_controls->success_rate_percents->setString(
          fmt::format("{}%", std::lround(percents)));
      m_controls->success_rate_progress->setPercentage(percents);
    }
}

void bim::axmol::app::lobby::closing()
{
  m_session_connection.disconnect();
}

void bim::axmol::app::lobby::apply_connected_state()
{
  m_controls->play_button->enable(
      m_context.get_session_handler()->connected());
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
