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

static constexpr std::string_view g_texture_scroll_tag = "scroll";

bim::axmol::widget::texture::texture(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_device_scale(context.device_scale)
  , m_scale(style.get_number("scale", 1))
  , m_scroll(ax::Vec2(-style.get_number("scroll-per-second.x", 0),
                      style.get_number("scroll-per-second.y", 0))
             * m_device_scale)
{
  ax::Sprite& s = *m_controls->sprite;

  s.setAnchorPoint(ax::Vec2(0, 0));
  s.setPosition(ax::Vec2(0, 0));
  s.getTexture()->setTexParameters(
      { ax::backend::SamplerFilter::LINEAR, ax::backend::SamplerFilter::LINEAR,
        ax::backend::SamplerAddressMode::REPEAT,
        ax::backend::SamplerAddressMode::REPEAT });
}

bim::axmol::widget::texture::~texture() = default;

void bim::axmol::widget::texture::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);

  ax::Sprite& s = *m_controls->sprite;
  const float final_scale = m_device_scale * m_scale;

  s.setTextureRect(
      ax::Rect(0, 0, size.width / final_scale, size.height / final_scale));
  s.setScale(final_scale);
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

void bim::axmol::widget::texture::onEnter()
{
  ax::Node::onEnter();

  if ((m_scroll.x == 0) && (m_scroll.y == 0))
    return;

  schedule(
      [=, this](float dt)
      {
        ax::Sprite& s = *m_controls->sprite;

        ax::Rect r = s.getTextureRect();
        r.origin += dt * m_scroll;

        s.setTextureRect(r);
      },
      g_texture_scroll_tag);
}

void bim::axmol::widget::texture::onExit()
{
  unschedule(g_texture_scroll_tag);

  ax::Node::onExit();
}
