// SPDX-License-Identifier: AGPL-3.0-only
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

#include <bim/axmol/widget/dynamic_factory.hpp>
#include <bim/axmol/widget/factory.hpp>
#include <bim/axmol/widget/instantiate_widgets.hpp>
#include <bim/axmol/widget/set_node_parents.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <iscool/memory/pimpl.impl.tpp>

#include <axmol/2d/Node.h>

#include <string_view>

// If no control is needed, we must still have the macro defined for this file
// to work properly.
#ifndef x_widget_controls
  #define x_widget_controls
#endif

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
    name->setName(#name);                                                     \
    all_nodes[#name] = std::move(node);                                       \
  }

    x_widget_controls
#undef x_widget

#define x_widget(type, name) #name,
        const std::string_view known[] = { "", x_widget_controls };
#undef x_widget

    bim::axmol::widget::instantiate_widgets(
        all_nodes, std::span(std::begin(known) + 1, std::end(known)), context,
        style);

    bim::axmol::widget::set_node_parents(all_nodes, style);
  }

  bim::axmol::widget::named_node_group all_nodes;

#define x_widget(type, name) type* name;
  x_widget_controls
#undef x_widget
#undef x_widget_controls
};
