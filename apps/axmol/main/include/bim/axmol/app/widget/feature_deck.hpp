// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/app/constant/game_feature_slot_count.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <axmol/2d/Node.h>

#include <cstdint>

namespace ax
{
  class Sprite;
}

namespace bim::axmol::app
{
  class feature_deck final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(feature_deck);

    feature_deck(const bim::axmol::widget::context& context,
                 const iscool::style::declaration& style);

    ~feature_deck();

    void features(bim::game::feature_flags flags);

    void setContentSize(const ax::Size& size) override;

  private:
    bool init() override;
    void onEnter() override;

    void update_display();

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls,
                                bim::app::g_game_feature_slot_count);

    const iscool::style::declaration* const
        m_style_bounds[bim::app::g_game_feature_slot_count + 1];
    const iscool::style::declaration* const
        m_style_display[bim::app::g_game_feature_slot_count + 1];

    ax::Sprite* m_slot[bim::app::g_game_feature_slot_count];
    bim::game::feature_flags m_feature_flags;
    std::uint8_t m_slot_count;
    bool m_dirty;
  };
}
