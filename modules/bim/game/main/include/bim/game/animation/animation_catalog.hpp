// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/animation/animation_specifications.hpp>

#include <vector>

namespace bim::game
{
  class animation_catalog
  {
  public:
    animation_catalog();
    ~animation_catalog();

    animation_id new_animation();
    const animation_specifications& get_animation(animation_id id) const;
    void replace_animation(animation_id id,
                           animation_specifications animation);

  private:
    std::vector<animation_specifications> m_animations;
  };
}
