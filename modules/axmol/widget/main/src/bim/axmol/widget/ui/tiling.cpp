// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/tiling.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <axmol/2d/Sprite.h>
#include <axmol/renderer/backend/Enums.h>

#define x_widget_scope bim::axmol::widget::tiling::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::Sprite, sprite)
#include <bim/axmol/widget/implement_controls_struct.hpp>

bim_implement_widget(bim::axmol::widget::tiling);

bim::axmol::widget::tiling::tiling(const bim::axmol::widget::context& context,
                                   const iscool::style::declaration& style)
  : m_controls(context, *style.get_declaration("widgets"))
  , m_texture_width_in_tiles(*style.get_number("width-in-tiles"))
  , m_node_width_in_tiles(1)
{
  ax::Sprite& s = *m_controls->sprite;

  s.setStretchEnabled(false);
  s.setAutoSize(false);
  s.setAnchorPoint(ax::Vec2(0.5, 0.5));
  s.getTexture()->setTexParameters(
      { ax::backend::SamplerFilter::LINEAR, ax::backend::SamplerFilter::LINEAR,
        ax::backend::SamplerAddressMode::REPEAT,
        ax::backend::SamplerAddressMode::REPEAT });
}

bim::axmol::widget::tiling::~tiling() = default;

void bim::axmol::widget::tiling::set_node_width_in_tiles(float w)
{
  m_node_width_in_tiles = w;
  update_display(getContentSize());
}

void bim::axmol::widget::tiling::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);
  update_display(size);
}

bool bim::axmol::widget::tiling::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeColorEnabled(true);
  setCascadeOpacityEnabled(true);

  add_group_as_children(*this, m_controls->all_nodes);

  return true;
}

void bim::axmol::widget::tiling::update_display(const ax::Size& size)
{
  ax::Sprite& s = *m_controls->sprite;
  const float scale = m_texture_width_in_tiles * size.width
                      / s.getTexture()->getPixelsWide()
                      / m_node_width_in_tiles;

  s.setContentSize(size);
  s.setPosition(size / 2);
  s.setTextureRect(ax::Rect(0, 0, size.width / scale, size.height / scale));
  s.setScale(scale);
}
