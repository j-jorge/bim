// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_observer.hpp>

namespace bim::axmol::input
{
  class touch_sink : public bim::axmol::input::touch_observer
  {
  private:
    void do_pressed(touch_event& touch) override;
    void do_moved(touch_event& touch) override;
    void do_released(touch_event& touch) override;
    void do_cancelled(touch_event& touch) override;
    void do_unplugged() override;
  };
}
