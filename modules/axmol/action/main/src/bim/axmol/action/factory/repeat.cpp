#include <bim/axmol/action/factory/repeat.hpp>

#include <bim/axmol/action/dynamic_factory.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/ActionInterval.h>

#include <cassert>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::repeat_from_style(const dynamic_factory& factory,
                                      const bim::axmol::colour_chart& colors,
                                      const iscool::style::declaration& style)
{
  const bim::axmol::ref_ptr<ax::FiniteTimeAction> action =
      factory.create(colors, *style.get_declaration("action"));

  assert(dynamic_cast<ax::ActionInterval*>(action.get()) != nullptr);
  ax::ActionInterval* const inner_action = (ax::ActionInterval*)action.get();

  // It seems that we cannot combine RepeatForever and TargetedAction, thus we
  // will just use a very large number of loops.
  //
  // From Axmol's documentation, the number of loops is limited to 2^30.
  const int loops = style.get_boolean("forever", false)
                        ? (1 << 30)
                        : (int)*style.get_number("count");

  return ax::Repeat::create(inner_action, loops);
}
