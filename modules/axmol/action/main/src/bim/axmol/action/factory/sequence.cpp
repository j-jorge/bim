// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/action/factory/sequence.hpp>

#include <bim/axmol/action/dynamic_factory.hpp>
#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/ActionInterval.h>

#include <algorithm>
#include <vector>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::sequence_from_style(
    const dynamic_factory& factory, const bim::axmol::colour_chart& colors,
    const iscool::style::declaration& style)
{
  std::vector<const iscool::style::declaration*> styles;
  styles.reserve(2);

  for (const iscool::style::declaration::declaration_map::value_type& entry :
       style.get_declarations())
    styles.push_back(&*entry.second);

  std::sort(styles.begin(), styles.end(),
            [](const iscool::style::declaration* a,
               const iscool::style::declaration* b)
            {
              return *a->get_number("index") < *b->get_number("index");
            });

  ax::Vector<ax::FiniteTimeAction*> actions(styles.size());

  for (const iscool::style::declaration* s : styles)
    actions.pushBack(
        static_cast<ax::FiniteTimeAction*>(factory.create(colors, *s).get()));

  ax::ActionInterval* sequence = ax::Sequence::create(actions);
  sequence = maybe_wrap_in_easing_function(*sequence, style);

  return sequence;
}
