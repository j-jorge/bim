/**
 * \file
 * This file use the X-macro idiom to implement a class with a given list of
 * widgets such that:
 *  - The listed widgets are available as members of the provided types.
 *  - Additional widgets can be instantiated dynamically from the style.
 *  - There is an `all_nodes` member of type `named_node_group` that collects
 *    all the widgets.
 *
 * Before including this header you must set the following macros:
 *   - `#define x_widget_scope` the scope within which the class is to be
 *     implemented. It may be empty but if it is not it must end with a pair of
 *     colons.
 *   - `#define x_widget_type_name` The unqualified name of the type to
 *     implement.
 *   - `#define x_widget_control x_widget(type, name)…` The list of * widgets
 *     to implement, with as many invocations of `x_widget` as needed, one for
 *     each widget of type `type` and name `name`.
 */

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/factory.hpp>
#include <bim/axmol/widget/named_node_group.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <iscool/style/declaration.hpp>

#include <string_view>

struct x_widget_scope x_widget_type_name
#undef x_widget_scope
{
  x_widget_type_name(const x_widget_type_name&) = delete;
  x_widget_type_name(const bim::axmol::widget::context& context,
                     const iscool::style::declaration& style)
#undef x_widget_type_name
  {
    // Initializing all members and filling the map of all nodes.
#define x_widget(type, name)                                                  \
  {                                                                           \
    bim::axmol::ref_ptr<type> node =                                          \
        bim::axmol::widget::factory<type>::create(                            \
            context, *style.get_declaration(#name));                          \
    name = node.get();                                                        \
    all_nodes[#name] = std::move(node);                                       \
  }

    x_widget_controls
#undef x_widget

#define x_widget(type, name) #name,
        const std::string_view known[] = { x_widget_controls };
#undef x_widget

    const std::string_view* const begin = std::begin(known);
    const std::string_view* const end = std::end(known);

    for (const iscool::style::declaration::declaration_map::value_type& d :
         style.get_declarations())
      if (std::find(begin, end, d.first) == end)
        {
          const iscool::optional<const std::string&> type =
              d.second->get_string("instantiate");

          if (type)
            all_nodes.emplace(
                d.first, context.factory.create(*type, context, *d.second));
        }
  }

  bim::axmol::widget::named_node_group all_nodes;

#define x_widget(type, name) type* name;
  x_widget_controls
#undef x_widget
#undef x_widget_controls
};
