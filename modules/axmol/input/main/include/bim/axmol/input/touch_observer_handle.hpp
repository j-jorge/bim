// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/touch_observer_pointer.hpp>

namespace bim::axmol::input
{
  /**
   * The handle is just a wrapper to instantiate the filter and to hide the
   * shared pointer from the owner.
   */
  template <typename T>
  class touch_observer_handle
  {
  public:
    touch_observer_handle();

    template <typename... Arg>
    explicit touch_observer_handle(Arg&&... args);
    ~touch_observer_handle();

    T* operator->() const;
    T& operator*() const;

    operator touch_observer_pointer() const;

  private:
    std::shared_ptr<T> m_observer;
  };
}
