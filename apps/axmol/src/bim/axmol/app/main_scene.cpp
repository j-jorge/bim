#include <bim/axmol/app/main_scene.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>

#define x_widget_scope bim::axmol::app::main_scene::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::Node, overlay) x_widget(ax::Node, main)
#include <bim/axmol/widget/implement_controls_struct.hpp>

// TODO: rm
#include <bim/axmol/style/apply_bounds.hpp>

bim::axmol::app::main_scene::main_scene(
    ax::Scene& scene, const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_scene(scene)
  , m_controls(context, *style.get_declaration("widgets"))
{
  bim::axmol::widget::add_group_as_children(scene, m_controls->all_nodes);

  // TODO: implement in axmol::widget with correct order.
  for (const auto& e : m_controls->all_nodes)
    {
      bim::axmol::style::apply_bounds(
          context.style_cache.get_bounds(
              *style.get_declaration("layout")->get_declaration(e.first)),
          *e.second, *e.second->getParent());
    }
}
