// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/message.hpp>

#include <bim/axmol/app/main_scene.hpp>

#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/key_sink.hpp>
#include <bim/axmol/input/observer/touch_sink.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

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
  , m_container(*m_controls->all_nodes.find("container")->second)
{
  m_inputs.attach_to_root(m_key_sink);
  m_inputs.attach_to_root(m_touch_sink);
  m_inputs.push_back(m_controls->yes_button->input_node());

  m_controls->yes_button->connect_to_clicked(
      [this]()
      {
        hide();
      });
}

bim::axmol::app::message_popup::~message_popup() = default;

void bim::axmol::app::message_popup::show(std::string_view message)
{
  m_controls->message->setString(message);
  m_context.get_main_scene()->add_in_overlays(m_container, m_inputs.root());
  bim::axmol::widget::apply_bounds(m_context.get_widget_context().style_cache,
                                   m_controls->all_nodes, m_style_bounds);
}

void bim::axmol::app::message_popup::hide()
{
  m_context.get_main_scene()->remove_from_overlays(m_container);
}
