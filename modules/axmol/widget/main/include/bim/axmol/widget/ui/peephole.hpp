// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <iscool/signals/declare_signal.hpp>

#include <axmol/2d/Node.h>

namespace ax
{
  class FiniteTimeAction;
}

namespace bim::axmol::widget
{
  class peephole final : public ax::Node
  {
    DECLARE_VOID_SIGNAL(shown, m_shown)

  public:
    bim_declare_widget_create_function(peephole);

    peephole(const bim::axmol::widget::context& context,
             const iscool::style::declaration& style);
    ~peephole();

    void prepare(const ax::Vec2& focus_world_position);
    void show();
    void reveal();

  private:
    class widgets;

  private:
    bool init() override;

    void scale_sprite(float from, float to, float f);

  private:
    bim_declare_controls_struct(controls, m_controls, 1);

    const float m_device_scale;
    const ax::Vec2 m_sprite_size;
    const float m_initial_scale;
    const float m_wait_scale;
    const float m_final_scale;

    const bim::axmol::ref_ptr<ax::FiniteTimeAction> m_action_show;
    const bim::axmol::ref_ptr<ax::FiniteTimeAction> m_action_reveal;

    ax::Vec2 m_focus;
  };
}
