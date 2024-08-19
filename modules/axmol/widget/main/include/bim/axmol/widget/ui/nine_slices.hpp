// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <axmol/2d/Node.h>

namespace bim::axmol::widget
{
  class nine_slices final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(nine_slices);

    nine_slices(const bim::axmol::widget::context& context,
                const iscool::style::declaration& style);
    ~nine_slices();

    void setContentSize(const ax::Size& size) override;

  private:
    class widgets;

  private:
    bool init() override;

  private:
    bim_declare_controls_struct(controls, m_controls, 1);
    ax::Vec2 m_minimum_size;
    const float m_device_scale;
  };
}
