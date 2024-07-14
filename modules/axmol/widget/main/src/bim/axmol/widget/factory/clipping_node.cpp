#include <bim/axmol/widget/factory/clipping_node.hpp>

#include <bim/axmol/widget/context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/ClippingNode.h>

bim::axmol::ref_ptr<ax::ClippingNode>
bim::axmol::widget::factory<ax::ClippingNode>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  ax::ClippingNode* const result = ax::ClippingNode::create();

  result->setAlphaThreshold(style.get_number("alpha-threshold", 0.5));

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
