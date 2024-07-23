#include <bim/axmol/action/factory/scale.hpp>

#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <iscool/optional.hpp>
#include <iscool/optional.impl.tpp>

#include <axmol/2d/ActionEase.h>
#include <axmol/2d/ActionInterval.h>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::scale_from_style(const iscool::style::declaration& style)
{
  ax::ActionInterval* scale = ax::ScaleTo::create(
      *style.get_number("duration"), *style.get_number("to"));

  const iscool::optional<const std::string&> ease = style.get_string("ease");

  if (ease)
    scale = wrap_in_easing_function(*scale, *ease);

  const iscool::optional<float> from = style.get_number("from");

  if (!from)
    return scale;

  return ax::Sequence::create(ax::ScaleTo::create(0, *from), scale, nullptr);
}
