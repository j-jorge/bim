// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/archive_storage.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class output_archive
  {
  public:
    explicit output_archive(archive_storage& storage);

    template <typename T>
    void operator()(const T& value) const;

    template <typename T>
    void operator()(entt::entity entity, const T& value) const;

  private:
    archive_storage& m_storage;
  };
}

#include <bim/game/output_archive.impl.hpp>
