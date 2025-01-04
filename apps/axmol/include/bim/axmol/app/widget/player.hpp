// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/game/component/player_direction_fwd.hpp>

#include <axmol/2d/Node.h>

#include <cstdint>

namespace bim::axmol::app
{
  class player final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(player);

    player(const bim::axmol::widget::context& context,
           const iscool::style::declaration& style);
    ~player();

    void set_direction(bim::game::player_direction d);

    void setContentSize(const ax::Size& size) override;

  private:
    bool init() override;

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 4);

    const iscool::style::declaration& m_style_bounds;
  };
}
