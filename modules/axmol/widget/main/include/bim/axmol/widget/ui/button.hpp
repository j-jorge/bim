// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/axmol/input/observer/tap_observer_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <iscool/signals/declare_signal.hpp>

#include <axmol/2d/Node.h>

namespace bim::axmol::widget
{
  class button final : public ax::Node
  {
    DECLARE_VOID_SIGNAL(clicked, m_clicked)

  public:
    bim_declare_widget_create_function(button);

    button(const bim::axmol::widget::context& context,
           const iscool::style::declaration& style);
    ~button();

    bim::axmol::input::node_reference input_node() const;

    void onEnter() override;
    void setContentSize(const ax::Size& size) override;

    void enable(bool enabled);
    void cancel_on_swipe(bool v);

  private:
    class widgets;

  private:
    bool init() override;

    void input_press();
    void input_release();
    void click();

    void update_bounds();
    void update_display();

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 0);
    const bim::axmol::ref_ptr<ax::Node> m_container;
    const bim::axmol::input::tap_observer_handle m_tap_observer;
    bim::axmol::input::tree m_inputs;

    const iscool::style::declaration& m_style_bounds;

    const iscool::style::declaration& m_style_pressed;
    const iscool::style::declaration& m_style_released;
    const iscool::style::declaration& m_style_disabled;

    const bim::axmol::ref_ptr<ax::Action> m_action_pressed;
    const bim::axmol::ref_ptr<ax::Action> m_action_released;

    const std::string m_sound;

    bool m_bounds_dirty;
    bool m_display_dirty;
    bool m_is_pressed;
  };
}
