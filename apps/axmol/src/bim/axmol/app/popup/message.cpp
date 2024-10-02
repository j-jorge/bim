// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/message.hpp>

#include <bim/axmol/app/popup/popup.hpp>

#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <axmol/2d/Label.h>

#define x_widget_scope bim::axmol::app::message_popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Label, message) x_widget(bim::axmol::widget::button, yes_button)

#include <bim/axmol/widget/implement_controls_struct.hpp>

bim::axmol::app::message_popup::message_popup(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
{
  m_controls->yes_button->connect_to_clicked(
      [this]()
      {
        m_popup->hide();
      });
}

bim::axmol::app::message_popup::~message_popup() = default;

void bim::axmol::app::message_popup::show(std::string_view message)
{
  m_controls->message->setString(message);

  m_popup->show(m_controls->all_nodes, m_style_bounds,
                m_controls->yes_button->input_node());
}
