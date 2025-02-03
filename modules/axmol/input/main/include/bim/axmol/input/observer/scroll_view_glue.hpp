// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_event.hpp>
#include <bim/axmol/input/touch_observer.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ax
{
  class Vec2;
}

namespace ax::extension
{
  class ScrollView;
}

namespace bim::axmol::input
{
  /**
   * Forward the inputs to Axmol's scroll view. The inputs are handled by our
   * input tree system.
   */
  class scroll_view_glue : public touch_observer
  {
  public:
    explicit scroll_view_glue(ax::extension::ScrollView& view);

    bool should_ignore_touches() const;
    bool contains_touch(const touch_event& touch) const;

  private:
    void do_pressed(const touch_event_view& touches) override;
    void do_moved(const touch_event_view& touches) override;
    void do_released(const touch_event_view& touches) override;
    void do_cancelled(const touch_event_view& touches) override;

    void categorize_moving_touch(touch_event& touch,
                                 std::vector<ax::Touch*>& began,
                                 std::vector<ax::Touch*>& moved);
    void categorize_released_touch(touch_event& touch,
                                   std::vector<ax::Touch*>& released,
                                   std::vector<ax::Touch*>& cancelled);

  private:
    ax::extension::ScrollView& m_view;

    std::unordered_map<int, ax::Vec2> m_touch_initial_position;
    std::unordered_set<int> m_active_touch;
  };
}
