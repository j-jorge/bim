#include <bim/axmol/widget/factory/layer_color.hpp>

#include <bim/axmol/widget/context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <axmol/2d/Layer.h>

bim::axmol::ref_ptr<ax::LayerColor>
bim::axmol::widget::factory<ax::LayerColor>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  ax::LayerColor* const result = ax::LayerColor::create();
  result->setIgnoreAnchorPointForPosition(false);

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
