#include <bim/axmol/widget/factory/node.hpp>

#include <bim/axmol/widget/context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <axmol/2d/Node.h>

bim::axmol::ref_ptr<ax::Node> bim::axmol::widget::factory<ax::Node>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  ax::Node* const result = ax::Node::create();

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
