// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/action/register_actions.hpp>

#include <bim/axmol/action/dynamic_factory.impl.hpp>
#include <bim/axmol/action/move.hpp>

#include <bim/axmol/action/factory/fade.hpp>
#include <bim/axmol/action/factory/repeat.hpp>
#include <bim/axmol/action/factory/rotate.hpp>
#include <bim/axmol/action/factory/scale.hpp>
#include <bim/axmol/action/factory/sequence.hpp>
#include <bim/axmol/action/factory/spawn.hpp>
#include <bim/axmol/action/factory/tint.hpp>
#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <axmol/2d/ActionInstant.h>
#include <axmol/2d/ActionInterval.h>

void bim::axmol::action::register_actions(dynamic_factory& factory)
{
#define register_action_1(name)                                               \
  factory.register_action(                                                    \
      #name,                                                                  \
      [](const colour_chart&, const iscool::style::declaration& style)        \
      {                                                                       \
        return name##_from_style(style);                                      \
      })
#define register_action_2(name)                                               \
  factory.register_action(#name,                                              \
                          [](const colour_chart& colours,                     \
                             const iscool::style::declaration& style)         \
                          {                                                   \
                            return name##_from_style(colours, style);         \
                          })
#define register_action_3(name)                                               \
  factory.register_action(#name,                                              \
                          [&factory](const colour_chart& colours,             \
                                     const iscool::style::declaration& style) \
                          {                                                   \
                            return name##_from_style(factory, colours,        \
                                                     style);                  \
                          })

  register_action_1(fade);
  register_action_3(repeat);
  register_action_1(rotate);
  register_action_1(scale);
  register_action_3(sequence);
  register_action_3(spawn);
  register_action_2(tint);

  factory.register_action(
      "hide",
      [](const colour_chart&, const iscool::style::declaration&)
      {
        return ax::Hide::create();
      });

  factory.register_action(
      "show",
      [](const colour_chart&, const iscool::style::declaration&)
      {
        return ax::Show::create();
      });

  factory.register_action(
      "move",
      [](const colour_chart&, const iscool::style::declaration& style)
      {
        return maybe_wrap_in_easing_function(*move::create(style), style);
      });

#undef register_action_3
#undef register_action_2
#undef register_action_1
}
