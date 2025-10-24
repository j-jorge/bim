// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/action/factory/rotate.hpp>

#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/ActionInterval.h>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::rotate_from_style(const iscool::style::declaration& style)
{
  const float duration = *style.get_number("duration");
  const iscool::optional<float> to = style.get_number("to");
  const iscool::optional<float> by = style.get_number("by");

  ax::ActionInterval* rotate =
      by ? (ax::ActionInterval*)ax::RotateBy::create(duration, *by)
         : (ax::ActionInterval*)ax::RotateTo::create(duration, *to);
  rotate = maybe_wrap_in_easing_function(*rotate, style);

  const iscool::optional<float> from = style.get_number("from");

  if (!from)
    return rotate;

  return ax::Sequence::create(ax::RotateTo::create(0, *from), rotate, nullptr);
}
