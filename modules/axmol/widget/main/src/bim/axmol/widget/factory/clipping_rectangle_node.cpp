// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/clipping_rectangle_node.hpp>

#include <bim/axmol/widget/context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <axmol/2d/ClippingRectangleNode.h>

bim::axmol::ref_ptr<ax::ClippingRectangleNode>
bim::axmol::widget::factory<ax::ClippingRectangleNode>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  ax::ClippingRectangleNode* const result =
      ax::ClippingRectangleNode::create();

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
