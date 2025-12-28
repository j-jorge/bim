// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/cell_edge_fwd.hpp>
#include <bim/game/cell_neighborhood_fwd.hpp>

#include <bim/table_2d.hpp>

#include <cstdint>
#include <span>
#include <vector>

namespace bim::game
{
  struct static_wall;
  struct fence;

  class arena
  {
  public:
    using static_wall_const_iterator =
        std::vector<static_wall>::const_iterator;

  public:
    arena();
    arena(std::uint8_t width, std::uint8_t height);
    arena(const arena& that) noexcept;
    arena(arena&& that) noexcept;
    ~arena();

    arena& operator=(const arena& that) noexcept;
    arena& operator=(arena&& that) noexcept;

    std::uint8_t width() const;
    std::uint8_t height() const;

    std::span<const static_wall> static_walls() const;

    bool is_static_wall(std::uint8_t x, std::uint8_t y) const;
    void set_static_wall(std::uint8_t x, std::uint8_t y, cell_neighborhood n);

    std::span<const fence> fences() const;

    cell_edge fences(std::uint8_t x, std::uint8_t y) const;
    void add_fence(std::uint8_t x, std::uint8_t y, cell_edge e);
    void remove_fence(std::uint8_t x, std::uint8_t y, cell_edge e);

  private:
    std::uint8_t m_width;
    std::uint8_t m_height;

    /// Static walls, they are never removed.
    table_2d<bool> m_is_static_wall;

    table_2d<cell_edge> m_fences;

    std::vector<static_wall> m_static_walls;
  };
}
