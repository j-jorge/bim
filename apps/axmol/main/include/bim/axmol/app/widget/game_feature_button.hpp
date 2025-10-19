// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/button_behavior.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <axmol/math/Color.h>

namespace ax
{
  class Sprite;
}

namespace bim::axmol::app
{
  class game_feature_button final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(game_feature_button);

    game_feature_button(const bim::axmol::widget::context& context,
                        const iscool::style::declaration& style);

    ~game_feature_button();

    iscool::signals::connection
    connect_to_clicked(std::function<void()> f) const;

    bim::axmol::input::node_reference input_node() const;

    void feature(bim::game::feature_flags f);

    void available(bool a);
    void price(int p);
    void affordable(bool a);

    void onEnter() override;
    void onExit() override;
    void setContentSize(const ax::Size& size) override;

  private:
    bool init() override;

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 2);
    const bim::axmol::ref_ptr<ax::Node> m_container;
    bim::axmol::widget::button_behavior m_behavior;

    const iscool::style::declaration& m_style_available;
    const iscool::style::declaration& m_style_unavailable;
    const iscool::style::declaration& m_style_feature;
    const iscool::style::declaration& m_style_no_feature;

    const ax::Color4B m_color_affordable;
    const ax::Color4B m_color_unaffordable;
  };
}
