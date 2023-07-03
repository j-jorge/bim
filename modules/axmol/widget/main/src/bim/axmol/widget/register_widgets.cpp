#include <bim/axmol/widget/register_widgets.hpp>

#include <bim/axmol/widget/dynamic_factory.impl.tpp>
#include <bim/axmol/widget/factory/node.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>

void bim::axmol::widget::register_widgets(dynamic_factory& factory)
{
#define register_type(type) factory.register_widget<type>(#type);

  register_type(ax::Node);
  register_type(ax::Sprite);

#undef register_type
}
