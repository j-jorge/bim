#include <bim/axmol/action/factory/spawn.hpp>

#include <bim/axmol/action/dynamic_factory.hpp>
#include <bim/axmol/action/factory/wrap_in_easing_function.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <iscool/optional.hpp>
#include <iscool/optional.impl.tpp>

#include <axmol/2d/ActionEase.h>
#include <axmol/2d/ActionInterval.h>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::spawn_from_style(const dynamic_factory& factory,
                                     const bim::axmol::colour_chart& colors,
                                     const iscool::style::declaration& style)
{
  ax::Vector<ax::FiniteTimeAction*> actions;
  actions.reserve(2);

  for (const iscool::style::declaration::declaration_map::value_type& entry :
       style.get_declarations())
    actions.pushBack(static_cast<ax::FiniteTimeAction*>(
        factory.create(colors, entry.second).get()));

  ax::ActionInterval* spawn = ax::Spawn::create(actions);

  const iscool::optional<const std::string&> ease = style.get_string("ease");

  if (ease)
    spawn = wrap_in_easing_function(*spawn, *ease);

  return spawn;
}
