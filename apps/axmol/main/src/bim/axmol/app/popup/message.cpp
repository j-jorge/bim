// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/message.hpp>

#include <bim/axmol/app/popup/popup.hpp>

#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Label.h>

#define x_widget_scope bim::axmol::app::message_popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Label, message)                                                \
      x_widget(bim::axmol::widget::button, yes_button)                        \
          x_widget(bim::axmol::widget::button, no_button)

#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::message_popup, ok, m_ok);

bim::axmol::app::message_popup::message_popup(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(*context.get_widget_context(),
               *style.get_declaration("widgets"))
  , m_yes_only_bounds(*style.get_declaration("yes-only-bounds"))
  , m_yes_no_bounds(*style.get_declaration("yes-no-bounds"))
  , m_yes_only_display(*style.get_declaration("yes-only-display"))
  , m_yes_no_display(*style.get_declaration("yes-no-display"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
{
  m_inputs.push_back(m_controls->yes_button->input_node());
  m_inputs.push_back(m_controls->no_button->input_node());

  m_controls->yes_button->connect_to_clicked(
      [this]()
      {
        m_popup->hide();
        m_ok();
      });
  m_controls->no_button->connect_to_clicked(
      [this]()
      {
        m_popup->hide();
      });
}

bim::axmol::app::message_popup::~message_popup() = default;

void bim::axmol::app::message_popup::show(std::string_view message)
{
  show(message, m_yes_only_display, m_yes_only_bounds);
}

void bim::axmol::app::message_popup::show_yes_no(std::string_view message)
{
  show(message, m_yes_no_display, m_yes_no_bounds);
}

void bim::axmol::app::message_popup::show(
    std::string_view message, const iscool::style::declaration& display,
    const iscool::style::declaration& bounds) const
{
  m_controls->message->setString(message);

  bim::axmol::widget::apply_display(
      m_context.get_widget_context()->style_cache, m_controls->all_nodes,
      display);

  m_popup->show(m_controls->all_nodes, bounds, m_inputs.root());
}
