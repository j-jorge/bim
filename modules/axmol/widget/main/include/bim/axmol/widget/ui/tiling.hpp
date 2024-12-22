// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <axmol/2d/Node.h>

namespace bim::axmol::widget
{
  class tiling final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(tiling);

    tiling(const bim::axmol::widget::context& context,
           const iscool::style::declaration& style);
    ~tiling();

    void set_node_width_in_tiles(float w);

    void setContentSize(const ax::Size& size) override;

  private:
    class widgets;

  private:
    bool init() override;

    void update_display(const ax::Size& size);

  private:
    bim_declare_controls_struct(controls, m_controls, 1);
    const float m_texture_width_in_tiles;
    float m_node_width_in_tiles;
  };
}
