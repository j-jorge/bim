// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/archive_storage.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class input_archive
  {
  public:
    explicit input_archive(const char* storage);

    template <typename T>
    void operator()(T& value);

    template <typename T>
    void operator()(entt::entity& entity, T& value);

  private:
    const char* m_cursor;
  };
}

#include <bim/game/input_archive.impl.hpp>
