#pragma once

#include <bim/axmol/input/key_event_view.hpp>

namespace bim::axmol::input
{
  class key_observer
  {
  public:
    key_observer();
    virtual ~key_observer();

    key_observer(const key_observer&) = delete;
    key_observer& operator=(const key_observer&) = delete;

    void pressed(const key_event_view& key);
    void released(const key_event_view& key);

  private:
    virtual void do_pressed(const key_event_view& key);
    virtual void do_released(const key_event_view& key);
  };
}
