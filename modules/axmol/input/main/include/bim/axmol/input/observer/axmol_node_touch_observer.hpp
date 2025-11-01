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
    bool contains_touch(touch_event& touch) const;

  private:
    bool do_may_process(touch_event& touch) const override;

    void do_pressed(touch_event& touch) override;
    void do_moved(touch_event& touch) override;
    void do_released(touch_event& touch) override;
    void do_cancelled(touch_event& touch) override;

    void do_unplugged() override;

  protected:
    const ax::Node& m_node;
  };
}
