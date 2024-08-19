// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/key_observer.hpp>

namespace bim::axmol::input
{
  class key_sink : public bim::axmol::input::key_observer
  {
  private:
    void do_pressed(const key_event_view& keys) override;
    void do_released(const key_event_view& keys) override;
  };
}
