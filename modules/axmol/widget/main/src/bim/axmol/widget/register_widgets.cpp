// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/register_widgets.hpp>

#include <bim/axmol/widget/dynamic_factory.impl.hpp>
#include <bim/axmol/widget/factory/clipping_node.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/factory/layer.hpp>
#include <bim/axmol/widget/factory/layer_color.hpp>
#include <bim/axmol/widget/factory/node.hpp>
#include <bim/axmol/widget/factory/scale_nine_sprite.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/ui/clipping_node.hpp>
#include <bim/axmol/widget/ui/nine_slices.hpp>
#include <bim/axmol/widget/ui/passive_node.hpp>
#include <bim/axmol/widget/ui/texture.hpp>

#include <axmol/2d/ClippingNode.h>
#include <axmol/2d/Label.h>
#include <axmol/2d/Layer.h>
#include <axmol/ui/UIScale9Sprite.h>

void bim::axmol::widget::register_widgets(dynamic_factory& factory)
{
#define register_type(type) factory.register_widget<type>(#type);

  register_type(ax::ClippingNode);
  register_type(ax::Label);
  register_type(ax::Layer);
  register_type(ax::LayerColor);
  register_type(ax::Node);
  register_type(ax::Sprite);
  register_type(ax::ui::Scale9Sprite);

  register_type(clipping_node);
  register_type(nine_slices);
  register_type(passive_node);
  register_type(texture);

#undef register_type
}
