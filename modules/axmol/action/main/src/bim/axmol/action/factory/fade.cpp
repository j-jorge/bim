#include <bim/axmol/action/factory/fade.hpp>

#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <iscool/optional.hpp>
#include <iscool/optional.impl.tpp>

#include <axmol/2d/ActionEase.h>
#include <axmol/2d/ActionInterval.h>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::fade_from_style(const iscool::style::declaration& style)
{
  ax::ActionInterval* fade = ax::FadeTo::create(*style.get_number("duration"),
                                                *style.get_number("to") * 255);

  const iscool::optional<const std::string&> ease = style.get_string("ease");

  if (ease)
    fade = wrap_in_easing_function(*fade, *ease);

  const iscool::optional<float> from = style.get_number("from");

  if (!from)
    return fade;

  return ax::Sequence::create(ax::FadeTo::create(0, *from), fade, nullptr);
}
