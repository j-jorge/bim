// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/screen/lobby.hpp>

#include <bim/axmol/app/popup/settings_popup.hpp>

#include <bim/axmol/widget/named_node_group.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/net/session_handler.hpp>

#include <iscool/signals/implement_signal.hpp>

#define x_widget_scope bim::axmol::app::lobby::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, settings_button)                       \
      x_widget(bim::axmol::widget::button, play_button)
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::lobby, play, m_play);

bim::axmol::app::lobby::lobby(const context& context,
                              const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_settings(new settings_popup(context, *style.get_declaration("settings")))
{
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
  return m_controls->all_nodes;
}

void bim::axmol::app::lobby::displayed()
{
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
