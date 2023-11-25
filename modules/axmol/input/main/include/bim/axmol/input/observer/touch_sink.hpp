#pragma once

#include <bim/axmol/input/touch_observer.hpp>

namespace bim::axmol::input
{
  class touch_sink : public bim::axmol::input::touch_observer
  {
  private:
    void do_pressed(const touch_event_view& touches) override;
    void do_moved(const touch_event_view& touches) override;
    void do_released(const touch_event_view& touches) override;
    void do_cancelled(const touch_event_view& touches) override;
  };
}
