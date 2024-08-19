// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_event.hpp>
#include <bim/axmol/input/touch_observer.hpp>

namespace ax
{
  class Node;
}

namespace bim::axmol::input
{
  class axmol_node_touch_observer : public touch_observer
  {
  public:
    explicit axmol_node_touch_observer(const ax::Node& node);

    bool should_ignore_touches() const;
    bool contains_touch(const touch_event& touch) const;

  private:
    void do_pressed(const touch_event_view& touches) override;
    void do_moved(const touch_event_view& touches) override;
    void do_released(const touch_event_view& touches) override;
    void do_cancelled(const touch_event_view& touches) override;

  protected:
    const ax::Node& m_node;
  };
}
