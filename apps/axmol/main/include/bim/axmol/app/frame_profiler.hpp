// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <vector>

namespace ax
{
  class EventListenerCustom;
}

namespace bim::axmol
{
  template <typename T>
  class ref_ptr;
}

namespace bim::axmol::app
{
  class frame_profiler
  {
  public:
    frame_profiler();
    frame_profiler(const frame_profiler&) = delete;
    ~frame_profiler();

  private:
    void after_loop();
    void before_update();
    void after_update();
    void before_draw();
    void after_draw();

  private:
    std::vector<bim::axmol::ref_ptr<ax::EventListenerCustom>>
        m_event_listeners;
  };
}
