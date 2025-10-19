// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/feature_deck.hpp>

#include <bim/axmol/app/widget/game_feature_button.hpp>

#include <bim/axmol/widget/context.hpp>

#include <bim/app/config.hpp>
#include <bim/app/preference/feature_flags.hpp>
#include <bim/app/preference/wallet.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>

#include <iscool/preferences/local_preferences.hpp>
#include <iscool/signals/implement_signal.hpp>

#define x_widget_scope bim::axmol::app::feature_deck::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(game_feature_button, feature_falling_blocks)                       \
      x_widget(game_feature_button, feature_shield)                           \
          x_widget(game_feature_button, feature_invisibility)                 \
              x_widget(game_feature_button, feature_fog_of_war)
#include <bim/axmol/widget/implement_controls_struct.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::feature_deck, clicked, m_clicked);

bim::axmol::app::feature_deck::feature_deck(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
{
  const auto configure_button = [this](game_feature_button& b,
                                       bim::game::feature_flags f) -> void
  {
    m_buttons[f] = &b;
    m_inputs.push_back(b.input_node());

    b.connect_to_clicked(
        [this, f]() -> void
        {
          m_clicked(f);
        });
  };

  configure_button(*m_controls->feature_falling_blocks,
                   bim::game::feature_flags::falling_blocks);
  configure_button(*m_controls->feature_shield,
                   bim::game::feature_flags::shield);
  configure_button(*m_controls->feature_invisibility,
                   bim::game::feature_flags::invisibility);
  configure_button(*m_controls->feature_fog_of_war,
                   bim::game::feature_flags::fog_of_war);
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

void bim::axmol::app::feature_deck::displaying()
{
  const iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  const bim::game::feature_flags enabled_features =
      bim::app::enabled_feature_flags(preferences);
  const bim::game::feature_flags available_features =
      bim::app::available_feature_flags(preferences);

  const std::int64_t coins = bim::app::coins_balance(preferences);

  for (bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    configure_feature_button(enabled_features, available_features, f, coins);
}

void bim::axmol::app::feature_deck::activated(bim::game::feature_flags feature)
{
  m_buttons[feature]->active(true);
}

void bim::axmol::app::feature_deck::deactivated(
    bim::game::feature_flags feature)
{
  m_buttons[feature]->active(false);
}

void bim::axmol::app::feature_deck::purchased(bim::game::feature_flags feature)
{
  m_buttons[feature]->available(true);

  const std::int64_t coins =
      bim::app::coins_balance(*m_context.get_local_preferences());

  for (bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    m_buttons[f]->affordable(
        m_context.get_config()->game_feature_price[feature] <= coins);
}

void bim::axmol::app::feature_deck::configure_feature_button(
    bim::game::feature_flags enabled_features,
    bim::game::feature_flags available_features,
    bim::game::feature_flags feature, std::int64_t coins_balance) const
{
  game_feature_button& button = *m_buttons[feature];

  if (!!(available_features & feature))
    {
      button.available(true);
      button.active(!!(enabled_features & feature));
    }
  else
    {
      button.available(false);
      button.active(true);
    }

  const std::int16_t price =
      m_context.get_config()->game_feature_price[feature];
  button.price(price);

  button.affordable(price <= coins_balance);
}
