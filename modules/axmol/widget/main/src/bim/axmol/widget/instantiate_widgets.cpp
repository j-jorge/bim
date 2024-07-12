#include <bim/axmol/widget/instantiate_widgets.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/dynamic_factory.hpp>

#include <iscool/style/declaration.hpp>

#include <algorithm>

void bim::axmol::widget::instantiate_widgets(
    named_node_group& nodes, std::span<const std::string_view> excluded,
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{

  for (const iscool::style::declaration::declaration_map::value_type& d :
       style.get_declarations())
    if (std::find(excluded.begin(), excluded.end(), d.first) == excluded.end())
      {
        const iscool::optional<const std::string&> type =
            d.second->get_string("instantiate");

        if (type)
          nodes.emplace(d.first,
                        context.factory.create(*type, context, *d.second));
      }
}
