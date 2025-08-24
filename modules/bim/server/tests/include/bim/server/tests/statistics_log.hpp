// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/tests/statistics_log_line.hpp>

#include <string>
#include <vector>

namespace bim::server::tests
{
  class statistics_log
  {
  public:
    statistics_log();
    ~statistics_log();

    statistics_log(const statistics_log&) = delete;
    statistics_log& operator=(const statistics_log&) = delete;

    std::vector<statistics_log_line> read_log_file() const;
    const std::string& log_file() const;

  private:
    std::string m_log_file;
  };
}
