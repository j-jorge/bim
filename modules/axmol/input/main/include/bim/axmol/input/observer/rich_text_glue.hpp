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
    void do_pressed(const touch_event_view& touches) override;
    void do_moved(const touch_event_view& touches) override;
    void do_released(const touch_event_view& touches) override;
    void do_cancelled(const touch_event_view& touches) override;

  protected:
    ax::ui::RichText& m_node;
    std::unordered_set<int> m_pressed;
  };
}
