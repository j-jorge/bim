// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_event_view.hpp>

namespace bim::axmol::input
{
  class touch_observer
  {
  public:
    touch_observer();
    virtual ~touch_observer();

    touch_observer(const touch_observer&) = delete;
    touch_observer& operator=(const touch_observer&) = delete;

    void pressed(const touch_event_view& touches);
    void moved(const touch_event_view& touches);
    void released(const touch_event_view& touches);
    void cancelled(const touch_event_view& touches);

  private:
    virtual void do_pressed(const touch_event_view& touches) = 0;
    virtual void do_moved(const touch_event_view& touches) = 0;
    virtual void do_released(const touch_event_view& touches) = 0;
    virtual void do_cancelled(const touch_event_view& touches) = 0;
  };
}
