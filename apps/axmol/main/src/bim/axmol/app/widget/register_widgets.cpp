// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/widget/register_widgets.hpp>

#include <bim/axmol/app/widget/feature_deck.hpp>

#include <bim/axmol/widget/dynamic_factory.impl.hpp>

void bim::axmol::app::register_widgets(
    bim::axmol::widget::dynamic_factory& factory)
{
#define register_type(type) factory.register_widget<type>(#type);

  register_type(bim::axmol::app::feature_deck);

#undef register_type
}
