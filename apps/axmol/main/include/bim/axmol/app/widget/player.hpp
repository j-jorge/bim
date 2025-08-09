// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/game/animation/animation_id.hpp>

#include <axmol/2d/Node.h>

#include <cstdint>
#include <unordered_map>

namespace bim::axmol::widget
{
  class animation_cache;
  struct animation;
}

namespace bim::game
{
  struct animation_state;
  struct player_animations;
}

namespace bim::axmol::app
{
  class player final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(player);

    player(const bim::axmol::widget::context& context,
           const iscool::style::declaration& style);

    ~player();

    void configure(const bim::axmol::widget::animation_cache& animations,
                   const bim::game::player_animations& player_animations,
                   std::uint8_t player_index);

    void set_animation(bool shield, const bim::game::animation_state& state);

    void setContentSize(const ax::Size& size) override;

  private:
    using animation_map =
        std::unordered_map<bim::game::animation_id,
                           const bim::axmol::widget::animation*>;

  private:
    bool init() override;

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 1);

    const iscool::style::declaration& m_style_bounds;

    animation_map m_animations;
    animation_map m_shield_animations;
  };
}
