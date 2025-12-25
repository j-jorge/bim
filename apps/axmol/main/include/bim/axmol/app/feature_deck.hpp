// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <bim/bit_map.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <cstdint>

namespace bim::axmol::widget
{
  class button;
  class context;
  class toggle;
}

namespace bim::app
{
  struct config;
}

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class legacy_game_feature_button;

  class feature_deck
  {
    DECLARE_SIGNAL(void(bim::game::feature_flags), clicked, m_clicked)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context*)(widget_context))         //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((const bim::app::config*)(config))                            //
            ),
        ic_context_no_properties);

  public:
    feature_deck(const context& context,
                 const iscool::style::declaration& style);
    ~feature_deck();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& display_nodes() const;

    void displaying();

    void activated(bim::game::feature_flags feature);
    void deactivated(bim::game::feature_flags feature);
    void purchased(bim::game::feature_flags feature);

  private:
    void configure_feature_button(bool enabled, bool available,
                                  bim::game::feature_flags feature,
                                  std::int64_t coins_balance) const;

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 4);
    bim::bit_map<bim::game::feature_flags, legacy_game_feature_button*>
        m_buttons;
  };
}
