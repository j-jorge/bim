// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/axmol/input/node_reference.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <axmol/2d/Node.h>
#include <axmol/math/Color.h>

#include <iscool/signals/connection.hpp>

#include <functional>

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

    void available(bool a);
    void price(int p);
    void affordable(bool a);
    void active(bool a);

    void setContentSize(const ax::Size& size) override;

  private:
    bool init() override;

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 2);

    const iscool::style::declaration& m_style_bounds;
    const iscool::style::declaration& m_style_available;
    const iscool::style::declaration& m_style_unavailable;

    const ax::Color4B m_color_affordable;
    const ax::Color4B m_color_unaffordable;
  };
}
