#include <bim/axmol/widget/register_widgets.hpp>

#include <bim/axmol/widget/dynamic_factory.impl.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/factory/layer.hpp>
#include <bim/axmol/widget/factory/layer_color.hpp>
#include <bim/axmol/widget/factory/node.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>

#include <axmol/2d/Label.h>
#include <axmol/2d/Layer.h>

void bim::axmol::widget::register_widgets(dynamic_factory& factory)
{
#define register_type(type) factory.register_widget<type>(#type);

  register_type(ax::Label);
  register_type(ax::Layer);
  register_type(ax::LayerColor);
  register_type(ax::Node);
  register_type(ax::Sprite);

#undef register_type
}
