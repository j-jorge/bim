// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/app/arena_display_config.hpp>

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <bim/game/cell_neighborhood.hpp>

#include <iscool/context.hpp>

#include <array>
#include <cstdint>
#include <vector>

namespace bim::axmol::widget
{
  class context;
}

namespace bim::game
{
  class contest;
}

namespace ax
{
  class Sprite;
  class SpriteFrame;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class fog_display
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                        //
            ((const bim::axmol::widget::context*)(widget_context))), //
        ic_context_no_properties);

  public:
    fog_display(const context& context,
                const iscool::style::declaration& style);
    ~fog_display();

    const bim::axmol::widget::named_node_group& display_nodes() const;

    void attached(const arena_display_config& display_config);
    void displaying(std::uint8_t displayed_player_index);
    void update(const bim::game::contest& contest);
    void closing();

  private:
    bim_declare_controls_struct(controls, m_controls, 1);

    std::vector<ax::Sprite*> m_sprite;
    std::array<bim::axmol::ref_ptr<ax::SpriteFrame>,
               bim::game::cell_neighborhood_layout_count>
        m_sprite_frame;
    std::vector<bim::axmol::ref_ptr<ax::SpriteFrame>> m_roll_in_frame;

    arena_display_config m_display_config;
    std::uint8_t m_displayed_player;
  };
}
