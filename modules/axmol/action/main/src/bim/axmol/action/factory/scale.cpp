// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/action/factory/scale.hpp>

#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/ActionInterval.h>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::scale_from_style(const iscool::style::declaration& style)
{
  ax::ActionInterval* scale = ax::ScaleTo::create(
      *style.get_number("duration"), *style.get_number("to"));
  scale = maybe_wrap_in_easing_function(*scale, style);

  const iscool::optional<float> from = style.get_number("from");

  if (!from)
    return scale;

  return ax::Sequence::create(ax::ScaleTo::create(0, *from), scale, nullptr);
}
