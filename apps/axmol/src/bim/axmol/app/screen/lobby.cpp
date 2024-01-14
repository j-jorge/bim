#include <bim/axmol/app/screen/lobby.hpp>

#include <bim/axmol/widget/ui/button.hpp>

#define x_widget_scope bim::axmol::app::lobby::
#define x_widget_type_name controls
#define x_widget_controls x_widget(bim::axmol::widget::button, play_button)
#include <bim/axmol/widget/implement_controls_struct.hpp>

bim::axmol::app::lobby::lobby(const context& context,
                              const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
{
  m_inputs.push_back(m_controls->play_button->input_node());
}

bim::axmol::app::lobby::~lobby() = default;

bim::axmol::input::node_reference bim::axmol::app::lobby::input_node() const
{
  return m_inputs.root();
}

const bim::axmol::widget::named_node_group&
bim::axmol::app::lobby::nodes() const
{
  return m_controls->all_nodes;
}
