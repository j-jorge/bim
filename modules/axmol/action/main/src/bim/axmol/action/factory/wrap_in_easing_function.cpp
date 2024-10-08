// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/assume.hpp>

#include <iscool/style/declaration.hpp>

#include <iscool/optional.hpp>
#include <iscool/optional.impl.tpp>

#include <axmol/2d/ActionEase.h>
#include <axmol/2d/ActionInterval.h>

[[nodiscard]] ax::ActionEase*
bim::axmol::action::wrap_in_easing_function(ax::ActionInterval& action,
                                            std::string_view function_name)
{
#define test_easing(bim_name, ax_name)                                        \
  if (function_name == #bim_name "-in")                                       \
    return ax::Ease##ax_name##In::create(&action);                            \
  if (function_name == #bim_name "-out")                                      \
    return ax::Ease##ax_name##Out::create(&action);                           \
  if (function_name == #bim_name "-in-out")                                   \
  return ax::Ease##ax_name##InOut::create(&action)

  test_easing(back, Back);
  test_easing(bounce, Bounce);
  test_easing(circle, CircleAction);
  test_easing(cubic, CubicAction);
  test_easing(exponential, Exponential);
  test_easing(quadratic, QuadraticAction);
  test_easing(quartic, QuarticAction);
  test_easing(quintic, QuinticAction);
  test_easing(sine, Sine);

  bim_assume(false);
  return nullptr;
}

[[nodiscard]] ax::ActionInterval*
bim::axmol::action::maybe_wrap_in_easing_function(
    ax::ActionInterval& action, const iscool::style::declaration& style)
{
  const iscool::optional<const std::string&> ease = style.get_string("ease");

  if (ease)
    return wrap_in_easing_function(action, *ease);

  return &action;
}
