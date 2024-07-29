#include <bim/axmol/action/factory/fade.hpp>

#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/ActionInterval.h>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::fade_from_style(const iscool::style::declaration& style)
{
  ax::ActionInterval* fade = ax::FadeTo::create(*style.get_number("duration"),
                                                *style.get_number("to") * 255);
  fade = maybe_wrap_in_easing_function(*fade, style);

  const iscool::optional<float> from = style.get_number("from");

  if (!from)
    return fade;

  return ax::Sequence::create(ax::FadeTo::create(0, *from), fade, nullptr);
}
