// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/registry.hpp>

#include <utility>

namespace bim::game
{
  /**
   * The game context is a second perspective on the game. Its data is either
   * constant over the duration of the game (e.g. animation state machines), or
   * it is deduced from the game. Typically, this data is not serialized with
   * the game state.
   */
  class context
  {
  public:
    template <typename T, typename... Args>
    T& create(Args&&... args)
    {
      return m_registry.ctx().emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    T& get() const
    {
      return m_registry.ctx().get<T>();
    }

    template <typename T>
    T& get()
    {
      return m_registry.ctx().get<T>();
    }

    template <typename T>
    T* try_get() const
    {
      return m_registry.ctx().find<T>();
    }

  private:
    entt::registry m_registry;
  };
}
