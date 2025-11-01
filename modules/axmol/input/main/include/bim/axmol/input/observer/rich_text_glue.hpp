// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/observer/axmol_node_touch_observer.hpp>

#include <unordered_set>

namespace ax::ui
{
  class RichText;
}

namespace bim::axmol::input
{
  class rich_text_glue : public axmol_node_touch_observer
  {
  public:
    explicit rich_text_glue(ax::ui::RichText& node);

  private:
    void do_pressed(touch_event& touch) override;
    void do_moved(touch_event& touch) override;
    void do_released(touch_event& touch) override;
    void do_cancelled(touch_event& touch) override;

    void do_unplugged() override;

  protected:
    ax::ui::RichText& m_node;
    std::unordered_set<int> m_pressed;
  };
}
