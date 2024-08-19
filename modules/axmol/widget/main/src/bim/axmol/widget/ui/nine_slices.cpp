// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/nine_slices.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/factory/scale_nine_sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <axmol/ui/UIScale9Sprite.h>

#define x_widget_scope bim::axmol::widget::nine_slices::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::ui::Scale9Sprite, sprite)
#include <bim/axmol/widget/implement_controls_struct.hpp>

bim_implement_widget(bim::axmol::widget::nine_slices);

bim::axmol::widget::nine_slices::nine_slices(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_device_scale(context.device_scale)
{
  const ax::Vec2 sprite_size = m_controls->sprite->getPreferredSize();
  const ax::Vec2 center_size = m_controls->sprite->getCapInsets().size;

  m_minimum_size = sprite_size - center_size;

  m_controls->sprite->setAnchorPoint(ax::Vec2(0.5, 0.5));
}

bim::axmol::widget::nine_slices::~nine_slices() = default;

void bim::axmol::widget::nine_slices::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  // Adapt the scale of the 9-slices such that it has the same aspect than
  // the reference size. Otherwise, depending on the resolution, the rounded
  // corners would be larger or thinner.

  ax::Size sprite_size = size / m_device_scale;
  ax::Size this_size;

  if (sprite_size.x < m_minimum_size.x)
    {
      sprite_size.x = m_minimum_size.x;
      this_size.x = m_minimum_size.x * m_device_scale;
    }
  else
    this_size.x = size.x;

  if (sprite_size.y < m_minimum_size.y)
    {
      sprite_size.y = m_minimum_size.y;
      this_size.y = m_minimum_size.y * m_device_scale;
    }
  else
    this_size.y = size.y;

  ax::Node::setContentSize(this_size);

  m_controls->sprite->setContentSize(sprite_size);
  m_controls->sprite->setPosition(this_size / 2);
  m_controls->sprite->setScale(m_device_scale);
}

bool bim::axmol::widget::nine_slices::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeOpacityEnabled(true);

  add_group_as_children(*this, m_controls->all_nodes);

  return true;
}
