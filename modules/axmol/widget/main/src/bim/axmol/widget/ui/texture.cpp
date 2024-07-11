#include <bim/axmol/widget/ui/texture.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <axmol/2d/Sprite.h>
#include <axmol/renderer/backend/Enums.h>

#define x_widget_scope bim::axmol::widget::texture::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::Sprite, sprite)
#include <bim/axmol/widget/implement_controls_struct.hpp>

bim_implement_widget(bim::axmol::widget::texture);

bim::axmol::widget::texture::texture(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_device_scale(context.device_scale)
{
  ax::Sprite& s = *m_controls->sprite;

  s.setAnchorPoint(ax::Vec2(0, 0));
  s.setPosition(ax::Vec2(0, 0));
  s.getTexture()->setTexParameters(
      { ax::backend::SamplerFilter::LINEAR, ax::backend::SamplerFilter::LINEAR,
        ax::backend::SamplerAddressMode::REPEAT,
        ax::backend::SamplerAddressMode::REPEAT });

  const ax::Vec2 scroll = ax::Vec2(-style.get_number("scroll-per-second.x", 0),
                                   style.get_number("scroll-per-second.y", 0))
                          * m_device_scale;

  if ((scroll.x != 0) || (scroll.y != 0))
    schedule(
        [=, &s](float dt)
        {
          ax::Rect r = s.getTextureRect();
          r.origin += dt * scroll;

          s.setTextureRect(r);
        },
        {});
}

bim::axmol::widget::texture::~texture() = default;

void bim::axmol::widget::texture::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);

  ax::Sprite& s = *m_controls->sprite;

  s.setTextureRect(ax::Rect(0, 0, size.width / m_device_scale,
                            size.height / m_device_scale));
  s.setScale(m_device_scale);
}

bool bim::axmol::widget::texture::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeColorEnabled(true);
  setCascadeOpacityEnabled(true);

  add_group_as_children(*this, m_controls->all_nodes);

  return true;
}
