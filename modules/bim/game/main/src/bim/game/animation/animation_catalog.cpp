// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/animation/animation_catalog.hpp>

#include <bim/assume.hpp>

bim::game::animation_catalog::animation_catalog()
  : m_animations(1)
{}

bim::game::animation_catalog::~animation_catalog() = default;

bim::game::animation_id bim::game::animation_catalog::new_animation()
{
  animation_id result = m_animations.size();
  m_animations.emplace_back();
  return result;
}

const bim::game::animation_specifications&
bim::game::animation_catalog::get_animation(animation_id id) const
{
  bim_assume(id > 0);
  bim_assume(id < m_animations.size());
  return m_animations[id];
}

void bim::game::animation_catalog::replace_animation(
    animation_id id, animation_specifications animation)
{
  bim_assume(id > 0);
  bim_assume(id < m_animations.size());
  m_animations[id] = std::move(animation);
}
