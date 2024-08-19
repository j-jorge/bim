// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/axmol/input/observer/soft_pad_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <iscool/signals/declare_signal.hpp>

#include <axmol/2d/Node.h>

namespace bim::axmol::widget
{
  class soft_pad final : public ax::Node
  {
    DECLARE_VOID_SIGNAL(pressed, m_pressed)

  public:
    bim_declare_widget_create_function(soft_pad);

    soft_pad(const bim::axmol::widget::context& context,
             const iscool::style::declaration& style);
    ~soft_pad();

    bim::axmol::input::node_reference input_node() const;

    void onEnter() override;
    void setContentSize(const ax::Size& size) override;

    void enable(bool enabled);

    ax::Vec2 direction() const;

  private:
    class widgets;

  private:
    bool init() override;

    /// Return true if a direction is pressed.
    bool update_display();

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 0);

    const bim::axmol::input::soft_pad_handle m_soft_pad_input;
    bim::axmol::input::tree m_inputs;

    const iscool::style::declaration& m_style_bounds;
    const iscool::style::declaration& m_style_up;
    const iscool::style::declaration& m_style_down;
    const iscool::style::declaration& m_style_left;
    const iscool::style::declaration& m_style_right;
    const iscool::style::declaration& m_style_off;
  };
}
