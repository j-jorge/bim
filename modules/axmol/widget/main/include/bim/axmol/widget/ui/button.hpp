// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/button_behavior.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

namespace bim::axmol::widget
{
  class button final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(button);

    button(const bim::axmol::widget::context& context,
           const iscool::style::declaration& style);
    ~button();

    iscool::signals::connection connect_to_clicked(std::function<void()> f);
    iscool::signals::connection connect_to_pressed(std::function<void()> f);

    bim::axmol::input::node_reference input_node() const;

    void onEnter() override;
    void onExit() override;
    void setContentSize(const ax::Size& size) override;

    void enable(bool enabled);
    void cancel_on_swipe(bool v);

  private:
    bool init() override;

  private:
    bim_declare_controls_struct(controls, m_controls, 0);
    const bim::axmol::ref_ptr<ax::Node> m_container;
    button_behavior m_behavior;
  };
}
