// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdio>
#include <filesystem>
#include <span>

namespace bim::game
{
  class contest_fingerprint;
  class player_action;

  class contest_timeline_writer
  {
  public:
    contest_timeline_writer();
    contest_timeline_writer(std::filesystem::path file_path,
                            const contest_fingerprint& contest);
    contest_timeline_writer(const contest_timeline_writer&) = delete;
    contest_timeline_writer(contest_timeline_writer&& that) noexcept;
    ~contest_timeline_writer();

    contest_timeline_writer&
    operator=(const contest_timeline_writer&) = delete;
    contest_timeline_writer&
    operator=(contest_timeline_writer&& that) noexcept;

    explicit operator bool() const;

    void push(std::span<player_action> actions);

  private:
    std::FILE* m_file;
  };
}
