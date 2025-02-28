// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/feature_deck.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/ui/button.hpp>
#include <bim/axmol/widget/ui/toggle.hpp>

#include <iscool/signals/implement_signal.hpp>

#define x_widget_scope bim::axmol::app::feature_deck::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::toggle, feature_falling_blocks)                \
      x_widget(bim::axmol::widget::toggle, feature_fog_of_war)                \
          x_widget(bim::axmol::widget::button, feature_extra_0)               \
              x_widget(bim::axmol::widget::button, feature_extra_1)           \
                  x_widget(bim::axmol::widget::button, feature_extra_2)       \
                      x_widget(bim::axmol::widget::button, feature_extra_3)
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::feature_deck, enabled, m_enabled);
IMPLEMENT_SIGNAL(bim::axmol::app::feature_deck, disabled, m_disabled);
IMPLEMENT_SIGNAL(bim::axmol::app::feature_deck, unavailable, m_unavailable);

bim::axmol::app::feature_deck::feature_deck(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_feature_flags{}
{
  m_inputs.push_back(m_controls->feature_falling_blocks->input_node());
  m_inputs.push_back(m_controls->feature_fog_of_war->input_node());
  m_inputs.push_back(m_controls->feature_extra_0->input_node());
  m_inputs.push_back(m_controls->feature_extra_1->input_node());
  m_inputs.push_back(m_controls->feature_extra_2->input_node());
  m_inputs.push_back(m_controls->feature_extra_3->input_node());

  const auto connect_toggle = [this](bim::axmol::widget::toggle& t,
                                     bim::game::feature_flags f) -> void
  {
    t.connect_to_clicked(
        [this, &t, f]() -> void
        {
          toggle_feature(t, f);
        });
  };

  connect_toggle(*m_controls->feature_falling_blocks,
                 bim::game::feature_flags::falling_blocks);
  connect_toggle(*m_controls->feature_fog_of_war,
                 bim::game::feature_flags::fog_of_war);

  const auto dispatch_unavailable = [this]() -> void
  {
    m_unavailable();
  };

  m_controls->feature_extra_0->connect_to_clicked(dispatch_unavailable);
  m_controls->feature_extra_1->connect_to_clicked(dispatch_unavailable);
  m_controls->feature_extra_2->connect_to_clicked(dispatch_unavailable);
  m_controls->feature_extra_3->connect_to_clicked(dispatch_unavailable);
}

bim::axmol::app::feature_deck::~feature_deck() = default;

bim::axmol::input::node_reference
bim::axmol::app::feature_deck::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::feature_deck::display_nodes() const
{
  return m_controls->all_nodes;
}

void bim::axmol::app::feature_deck::displaying(
    bim::game::feature_flags enabled_features,
    bim::game::feature_flags available_features)
{
  m_feature_flags = enabled_features;

  configure_feature_button(*m_controls->feature_falling_blocks,
                           *m_controls->feature_extra_0, available_features,
                           bim::game::feature_flags::falling_blocks);
  configure_feature_button(*m_controls->feature_fog_of_war,
                           *m_controls->feature_extra_1, available_features,
                           bim::game::feature_flags::fog_of_war);
}

bim::game::feature_flags bim::axmol::app::feature_deck::features() const
{
  return m_feature_flags;
}

void bim::axmol::app::feature_deck::configure_feature_button(
    bim::axmol::widget::toggle& state_toggle,
    bim::axmol::widget::button& unavailable_button,
    bim::game::feature_flags available_features,
    bim::game::feature_flags feature) const
{
  const bool available = !!(feature & available_features);

  state_toggle.setVisible(available);
  unavailable_button.setVisible(!available);

  if (available)
    state_toggle.set_state(!!(m_feature_flags & feature));
}

void bim::axmol::app::feature_deck::toggle_feature(
    bim::axmol::widget::toggle& t, bim::game::feature_flags f)
{
  m_feature_flags = m_feature_flags ^ f;

  const bool enabled = !!(m_feature_flags & f);
  t.set_state(enabled);

  if (enabled)
    m_enabled(f);
  else
    m_disabled(f);
}
