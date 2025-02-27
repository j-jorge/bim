// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/peephole.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/factory/sprite.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <axmol/2d/Sprite.h>
#include <axmol/renderer/backend/Enums.h>

#define x_widget_scope bim::axmol::widget::peephole::
#define x_widget_type_name controls
#define x_widget_controls x_widget(ax::Sprite, sprite)
#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/ActionEase.h>
#include <axmol/2d/ActionInstant.h>
#include <axmol/2d/ActionInterval.h>

IMPLEMENT_SIGNAL(bim::axmol::widget::peephole, shown, m_shown);

bim_implement_widget(bim::axmol::widget::peephole);

bim::axmol::widget::peephole::peephole(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
  : m_controls(context, style.get_declaration_or_empty("widgets"))
  , m_device_scale(context.device_scale)
  , m_sprite_size(m_controls->sprite->getContentSize())
  , m_initial_scale(style.get_number("initial-scale", 1))
  , m_wait_scale(style.get_number("wait-scale", 1))
  , m_final_scale(style.get_number("final-scale", 1))
  , m_action_show(ax::Sequence::create(
        ax::EaseBackOut::create(ax::ActionFloat::create(
            style.get_number("scale-in-seconds", 1), 0, 1,
            [this](float f)
            {
              scale_sprite(m_initial_scale, m_wait_scale, f);
            })),
        ax::CallFunc::create(
            [this]() -> void
            {
              m_shown();
            }),
        nullptr))
  , m_action_reveal(ax::EaseCubicActionOut::create(ax::Spawn::create(
        ax::FadeOut::create(style.get_number("fade-out-seconds", 1)),
        ax::ActionFloat::create(style.get_number("fade-out-seconds", 1), 0, 1,
                                [this](float f) -> void
                                {
                                  scale_sprite(m_wait_scale, m_final_scale, f);
                                }),
        nullptr)))
{
  ax::Sprite& sprite = *m_controls->sprite;

  sprite.setAnchorPoint(ax::Vec2(0, 0));
  sprite.setPosition(ax::Vec2(0, 0));
  sprite.getTexture()->setTexParameters(
      { ax::backend::SamplerFilter::LINEAR, ax::backend::SamplerFilter::LINEAR,
        ax::backend::SamplerAddressMode::CLAMP_TO_EDGE,
        ax::backend::SamplerAddressMode::CLAMP_TO_EDGE });
}

bim::axmol::widget::peephole::~peephole() = default;

void bim::axmol::widget::peephole::prepare(
    const ax::Vec2& focus_world_position)
{
  m_focus = convertToNodeSpace(focus_world_position);

  // m_focus is used in texture coordinates, thus increasing row index when
  // going down visually.
  m_focus.y = getContentSize().y - m_focus.y;

  scale_sprite(m_initial_scale, m_wait_scale, 0);
  m_controls->sprite->setOpacity(255);
}

void bim::axmol::widget::peephole::show()
{
  ax::Sprite& sprite = *m_controls->sprite;

  sprite.stopAllActions();
  sprite.runAction(m_action_show.get());
}

void bim::axmol::widget::peephole::reveal()
{
  ax::Sprite& sprite = *m_controls->sprite;

  sprite.stopAllActions();
  sprite.runAction(m_action_reveal.get());
}

bool bim::axmol::widget::peephole::init()
{
  if (!ax::Node::init())
    return false;

  setAnchorPoint(ax::Vec2(0.5, 0.5));
  setCascadeColorEnabled(true);
  setCascadeOpacityEnabled(true);

  add_group_as_children(*this, m_controls->all_nodes);

  return true;
}

void bim::axmol::widget::peephole::scale_sprite(float from, float to, float f)
{
  ax::Sprite& sprite = *m_controls->sprite;

  const float scale =
      std::max(m_device_scale * ((1 - f) * from + f * to), 0.0001f);
  const ax::Vec2 origin = m_focus / scale - m_sprite_size / 2;
  const ax::Vec2 size = getContentSize() / scale;

  sprite.setTextureRect(ax::Rect(-origin, size));
  sprite.setScale(scale);
}
