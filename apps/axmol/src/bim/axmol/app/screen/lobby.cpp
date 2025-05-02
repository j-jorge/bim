// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/lobby.hpp>

#include <bim/axmol/app/part/wallet.hpp>
#include <bim/axmol/app/popup/debug_popup.hpp>
#include <bim/axmol/app/popup/settings_popup.hpp>

#include <bim/axmol/input/observer/tap_observer.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>
#include <bim/axmol/widget/merge_named_node_groups.hpp>
#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/net/session_handler.hpp>

#include <iscool/signals/implement_signal.hpp>

#define x_widget_scope bim::axmol::app::lobby::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, settings_button)                       \
      x_widget(bim::axmol::widget::button, play_button)                       \
          x_widget(bim::axmol::widget::button, debug_button)                  \
              x_widget(ax::Node, debug_activator)
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::lobby, play, m_play);

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

  m_inputs.push_back(m_wallet->input_node());

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
