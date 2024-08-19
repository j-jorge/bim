// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <axmol/2d/Node.h>

namespace bim::axmol::widget
{
  class clipping_node final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(clipping_node);

    clipping_node(const bim::axmol::widget::context& context,
                  const iscool::style::declaration& style);
    ~clipping_node();

    void setContentSize(const ax::Size& size) override;

  private:
    class widgets;

  private:
    bool init() override;

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 1);
    const iscool::style::declaration& m_style_bounds;
    ax::Node& m_stencil;
  };
}
