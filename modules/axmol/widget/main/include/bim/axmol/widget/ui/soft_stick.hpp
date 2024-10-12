// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/axmol/input/observer/soft_stick_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <axmol/2d/Node.h>

namespace bim::axmol::widget
{
  class soft_stick final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(soft_stick);

    soft_stick(const bim::axmol::widget::context& context,
               const iscool::style::declaration& style);
    ~soft_stick();

    void set_layout_on_the_left(bool left);

    bim::axmol::input::node_reference input_node() const;

    void onEnter() override;
    void setContentSize(const ax::Size& size) override;

    void enable(bool enabled);

    const ax::Vec2& drag() const;

  private:
    class widgets;

  private:
    bool init() override;

    void update_display();
    void move_stick_to(const ax::Vec2& world_position);

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 1);

    const bim::axmol::input::soft_stick_handle m_soft_stick_input;
    bim::axmol::input::tree m_inputs;

    const iscool::style::declaration& m_style_bounds_left;
    const iscool::style::declaration& m_style_bounds_right;

    ax::Vec2 m_original_stick_world_position;

    bool m_on_the_left;
  };
}
