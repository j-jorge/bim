// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdio>

namespace bim::game
{
  class contest_fingerprint;
  class player_action;

  class contest_timeline_writer
  {
  public:
    contest_timeline_writer();
    contest_timeline_writer(std::FILE* file,
                            const contest_fingerprint& contest);
    contest_timeline_writer(const contest_timeline_writer&) = delete;
    contest_timeline_writer(contest_timeline_writer&& that) noexcept;
    ~contest_timeline_writer();

    contest_timeline_writer&
    operator=(const contest_timeline_writer&) = delete;
    contest_timeline_writer&
    operator=(contest_timeline_writer&& that) noexcept;

    explicit operator bool() const;

    void push(const entt::registry& registry);

  private:
    std::FILE* m_file;
    std::uint8_t m_player_count;
  };
}
