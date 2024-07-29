#include <bim/axmol/action/dynamic_factory.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/factory/dynamic_factory.impl.tpp>

#include <iscool/style/declaration.hpp>

#include <iscool/optional.hpp>
#include <iscool/optional.impl.tpp>

#include <axmol/2d/Action.h>

bim::axmol::ref_ptr<ax::FiniteTimeAction>
bim::axmol::action::dynamic_factory::create(
    const colour_chart& colors, const iscool::style::declaration& style) const
{
  const iscool::optional<const std::string&> name =
      style.get_string("instantiate");

  if (!name)
    return {};

  return m_factory.create_by_typename(*name, colors, style);
}
