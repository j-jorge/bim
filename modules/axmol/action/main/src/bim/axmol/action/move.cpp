// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/action/move.hpp>

#include <bim/assume.hpp>

#include <iscool/style/declaration.hpp>

bim::axmol::action::move*
bim::axmol::action::move::create(const iscool::style::declaration& style)
{
  move* const result = new move(style);
  result->autorelease();

  if (result->init())
    return result;

  delete result;

  return nullptr;
}

bim::axmol::action::move::~move() = default;

void bim::axmol::action::move::startWithTarget(ax::Node* target)
{
  bim_assume(target != nullptr);
  bim_assume(target->getParent() != nullptr);

  ax::MoveBy::startWithTarget(target);

  const ax::Vec2 parent_size = target->getParent()->getContentSize();
  ax::Vec2 initial_position = target->getPosition();
  ax::Vec2 final_position = initial_position;

  const iscool::optional<float> from_x = m_style.get_number("from.x");

  if (from_x)
    initial_position.x = *from_x * parent_size.x;

  const iscool::optional<float> from_y = m_style.get_number("from.y");

  if (from_y)
    initial_position.y = *from_y * parent_size.y;

  const iscool::optional<float> to_x = m_style.get_number("to.x");

  if (to_x)
    final_position.x = *to_x * parent_size.x;

  const iscool::optional<float> to_y = m_style.get_number("to.y");

  if (to_y)
    final_position.y = *to_y * parent_size.y;

  const iscool::optional<float> by_x = m_style.get_number("by.x");

  if (by_x)
    {
      if (to_x)
        initial_position.x = final_position.x - *by_x * parent_size.x;
      else
        final_position.x = initial_position.x + *by_x * parent_size.x;
    }

  const iscool::optional<float> by_y = m_style.get_number("by.y");

  if (by_y)
    {
      if (to_y)
        initial_position.y = final_position.y - *by_y * parent_size.y;
      else
        final_position.y = initial_position.y + *by_y * parent_size.y;
    }

  target->setPosition(initial_position);

  _startPosition = ax::Vec3(initial_position.x, initial_position.y, 0);
  _previousPosition = _startPosition;

  const ax::Vec2 delta = final_position - initial_position;
  _positionDelta = ax::Vec3(delta.x, delta.y, 0);
}

bim::axmol::action::move::move(const iscool::style::declaration& style)
  : m_style(style)
{}

bool bim::axmol::action::move::init()
{
  // Use zero for the delta position, it will be overriden in startWithTarget.
  return initWithDuration(*m_style.get_number("duration"), ax::Vec2::ZERO);
}
