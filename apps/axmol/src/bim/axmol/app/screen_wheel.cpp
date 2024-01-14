#include <bim/axmol/app/screen_wheel.hpp>

#include <bim/axmol/app/screen/lobby.hpp>

#define x_widget_scope bim::axmol::app::screen_wheel::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(ax::Node, lobby) x_widget(ax::Node, matchmaking)
#include <bim/axmol/widget/implement_controls_struct.hpp>

bim::axmol::app::screen_wheel::screen_wheel(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_main_container(ax::Node::create())
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_lobby(new lobby(m_context, *style.get_declaration("lobby")))
{
  m_context.get_main_scene()->add_in_main_canvas(*m_main_container,
                                                 m_inputs.root());

  bim::axmol::widget::add_group_as_children(*m_main_container,
                                            m_controls->all_nodes);
  bim::axmol::widget::apply_bounds(context.get_widget_context().style_cache,
                                   m_controls->all_nodes,
                                   *style.get_declaration("bounds"));

  bim::axmol::widget::add_group_as_children(*m_controls->lobby,
                                            m_lobby->nodes());
  bim::axmol::widget::apply_bounds(context.get_widget_context().style_cache,
                                   m_lobby->nodes(),
                                   *style.get_declaration("lobby-bounds"));

  m_controls->matchmaking->removeFromParent();

  m_inputs.push_back(m_lobby->input_node());
}

bim::axmol::app::screen_wheel::~screen_wheel() = default;
