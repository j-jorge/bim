#include <bim/axmol/action/factory/tint.hpp>

#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/axmol/colour_chart.hpp>
#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <iscool/optional.hpp>
#include <iscool/optional.impl.tpp>

#include <axmol/2d/ActionEase.h>
#include <axmol/2d/ActionInterval.h>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::tint_from_style(const bim::axmol::colour_chart& colors,
                                    const iscool::style::declaration& style)
{
  const ax::Color4B to = colors.to_color_4b(*style.get_string("to"));
  bool needs_fade = to.a != 255;
  const float duration = *style.get_number("duration");

  ax::ActionInterval* tint = ax::TintTo::create(duration, ax::Color3B(to));

  ax::Vector<ax::FiniteTimeAction*> actions;
  actions.reserve(4);

  const iscool::optional<const std::string&> from_string =
      style.get_string("from");

  if (from_string)
    {
      const ax::Color4B from = colors.to_color_4b(*from_string);
      actions.pushBack(ax::TintTo::create(0, ax::Color3B(from)));

      needs_fade |= (from.a != 255);

      if (needs_fade)
        actions.pushBack(ax::FadeTo::create(0, from.a));
    }

  if (needs_fade)
    tint =
        ax::Spawn::create(ax::FadeTo::create(duration, to.a), tint, nullptr);

  const iscool::optional<const std::string&> ease = style.get_string("ease");

  if (ease)
    tint = wrap_in_easing_function(*tint, *ease);

  actions.pushBack(tint);

  return ax::Sequence::create(actions);
}
