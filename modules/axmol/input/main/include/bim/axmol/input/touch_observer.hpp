// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_event.hpp>

namespace bim::axmol::input
{
  class touch_observer
  {
  public:
    touch_observer();
    virtual ~touch_observer();

    touch_observer(const touch_observer&) = delete;
    touch_observer& operator=(const touch_observer&) = delete;

    bool may_process(touch_event& touch) const;

    void pressed(touch_event& touch);
    void moved(touch_event& touch);
    void released(touch_event& touch);
    void cancelled(touch_event& touch);

    void unplugged();

  private:
    virtual bool do_may_process(touch_event& touch) const;

    virtual void do_pressed(touch_event& touch) = 0;
    virtual void do_moved(touch_event& touch) = 0;
    virtual void do_released(touch_event& touch) = 0;
    virtual void do_cancelled(touch_event& touch) = 0;

    virtual void do_unplugged() = 0;
  };
}
