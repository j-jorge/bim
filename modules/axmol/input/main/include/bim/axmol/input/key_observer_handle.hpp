#pragma once

#include <bim/axmol/input/key_observer_pointer.hpp>

namespace bim::axmol::input
{
  /**
   * The handle is just a wrapper to instantiate the filter and to hide the
   * shared pointer from the owner.
   */
  template <typename T>
  class key_observer_handle
  {
  public:
    template <typename... Arg>
    explicit key_observer_handle(Arg&&... args);
    ~key_observer_handle();

    T* operator->() const;
    T& operator*() const;

    operator key_observer_pointer() const;

  private:
    std::shared_ptr<T> m_observer;
  };
}
