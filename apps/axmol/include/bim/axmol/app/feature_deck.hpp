// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/game/feature_flags.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace bim::axmol::widget
{
  class button;
  class context;
  class toggle;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class feature_deck
  {
    DECLARE_SIGNAL(void(bim::game::feature_flags), enabled, m_enabled)
    DECLARE_SIGNAL(void(bim::game::feature_flags), disabled, m_disabled)
    DECLARE_VOID_SIGNAL(unavailable, m_unavailable)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties( //
            ((const bim::axmol::widget::context&)(widget_context))),
        ic_context_no_properties);

  public:
    feature_deck(const context& context,
                 const iscool::style::declaration& style);
    ~feature_deck();

    bim::axmol::input::node_reference input_node() const;
    const bim::axmol::widget::named_node_group& display_nodes() const;

    void displaying(bim::game::feature_flags enabled_features,
                    bim::game::feature_flags available_features);

    bim::game::feature_flags features() const;

  private:
    void
    configure_feature_button(bim::axmol::widget::toggle& state_toggle,
                             bim::axmol::widget::button& unavailable_button,
                             bim::game::feature_flags available_features,
                             bim::game::feature_flags feature) const;

    void toggle_feature(bim::axmol::widget::toggle& t,
                        bim::game::feature_flags f);

  private:
    bim::axmol::input::tree m_inputs;
    bim_declare_controls_struct(controls, m_controls, 6);
    bim::game::feature_flags m_feature_flags;
  };
}
