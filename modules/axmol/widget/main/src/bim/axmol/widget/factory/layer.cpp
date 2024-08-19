// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/layer.hpp>

#include <bim/axmol/widget/context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <axmol/2d/Layer.h>

bim::axmol::ref_ptr<ax::Layer> bim::axmol::widget::factory<ax::Layer>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  ax::Layer* const result = ax::Layer::create();

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
