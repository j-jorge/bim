// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <type_traits>

namespace bim::axmol
{
  /**
   * Smart pointer to automatically handle the retain/release calls on a an
   * ax::Object.
   *
   * We don't use ax::RefPtr because it cannot be declared with a
   * forward-declared type as the template argument, and because it does not
   * handle implicit conversions (e.g. RefPtr<ax::Node> xyz =
   * RefPtr<ax::Sprite>() does not compile.
   */
  template <typename T>
  class ref_ptr
  {
    template <typename U>
    friend class ref_ptr;

  public:
    ref_ptr() noexcept;
    ref_ptr(std::nullptr_t) noexcept;
    ref_ptr(T* p) noexcept;
    ref_ptr(const ref_ptr<T>& p) noexcept;
    ref_ptr(ref_ptr<T>&& p) noexcept;

    template <typename U>
      requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
    ref_ptr(U* p) noexcept;

    template <typename U>
      requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
    ref_ptr(const ref_ptr<U>& p) noexcept;

    template <typename U>
      requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
    ref_ptr(ref_ptr<U>&& p) noexcept;

    ~ref_ptr();

    ref_ptr& operator=(std::nullptr_t) noexcept;

    explicit operator bool() const
    {
      return m_ptr != nullptr;
    }

    template <typename U>
      requires std::is_base_of_v<T, U>
    ref_ptr& operator=(U* p) noexcept;

    ref_ptr& operator=(const ref_ptr& that) noexcept;
    ref_ptr& operator=(ref_ptr&& that) noexcept;

    template <typename U>
      requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
    ref_ptr& operator=(const ref_ptr<U>& that) noexcept;

    template <typename U>
      requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
    ref_ptr& operator=(ref_ptr<U>&& that) noexcept;

    template <typename U>
      requires std::is_base_of_v<T, U>
    friend bool operator==(const ref_ptr<T>& self, U* that)
    {
      return self.m_ptr == that;
    }

    template <typename U>
      requires std::is_base_of_v<T, U>
    friend bool operator==(const ref_ptr<T>& self, const ref_ptr<U>& that)
    {
      return self.m_ptr == that.get();
    }

    template <typename U>
      requires std::is_base_of_v<T, U>
    friend bool operator!=(const ref_ptr<T>& self, U* that)
    {
      return self.m_ptr != that;
    }

    template <typename U>
      requires std::is_base_of_v<T, U>
    friend bool operator!=(const ref_ptr<T>& self, const ref_ptr<U>& that)
    {
      return self.m_ptr != that.get();
    }

    T& operator*() const;
    T* operator->() const;

    T* get() const;

  private:
    T* m_ptr;
  };
}
