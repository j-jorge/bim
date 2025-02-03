// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_widget_create_function.hpp>
#include <bim/axmol/widget/named_node_group.hpp>

#include <axmol/2d/Node.h>

namespace bim::axmol::widget
{
  class passive_node final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(passive_node);

    passive_node(const bim::axmol::widget::context& context,
                 const iscool::style::declaration& style);
    ~passive_node();

    void onEnter() override;
    void setContentSize(const ax::Size& size) override;

    void fill(const named_node_group& nodes,
              const iscool::style::declaration& bounds);

  private:
    void update_bounds();

  private:
    const bim::axmol::widget::context& m_context;

    named_node_group m_nodes;
    const iscool::style::declaration* m_style_bounds;

    bool m_dirty;
  };
}
