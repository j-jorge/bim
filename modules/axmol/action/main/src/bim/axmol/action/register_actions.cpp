#include <bim/axmol/action/register_actions.hpp>

#include <bim/axmol/action/dynamic_factory.impl.hpp>

#include <bim/axmol/action/factory/fade.hpp>
#include <bim/axmol/action/factory/scale.hpp>
#include <bim/axmol/action/factory/spawn.hpp>
#include <bim/axmol/action/factory/tint.hpp>

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
  register_action_1(scale);
  register_action_2(tint);
  register_action_3(spawn);

#undef register_action_3
#undef register_action_2
#undef register_action_1
}
