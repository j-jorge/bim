// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <memory>

namespace bim
{
  template <typename T>
  class table_2d
  {
  public:
    table_2d();
    table_2d(std::size_t w, std::size_t h);
    table_2d(std::size_t w, std::size_t h, T&& init);

    table_2d(const table_2d<T>& that);
    table_2d(table_2d<T>&& that) noexcept;

    table_2d& operator=(const table_2d<T>& that);
    table_2d& operator=(table_2d<T>&& that) noexcept;

    ~table_2d();

    T& operator()(std::size_t x, std::size_t y);
    const T& operator()(std::size_t x, std::size_t y) const;

    [[nodiscard]] std::size_t width() const;
    [[nodiscard]] std::size_t height() const;

    void resize(std::size_t w, std::size_t h);
    void fill(const T& v);

    [[nodiscard]] T* begin();
    [[nodiscard]] T* begin() const;

    [[nodiscard]] T* end();
    [[nodiscard]] T* end() const;

  private:
    std::size_t m_width;
    std::size_t m_height;
    std::unique_ptr<T[]> m_data;
  };
}
