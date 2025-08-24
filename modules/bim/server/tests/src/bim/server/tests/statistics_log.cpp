// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/statistics_log.hpp>

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <unistd.h>

#include <gtest/gtest.h>

bim::server::tests::statistics_log::statistics_log()
{
  std::string t =
      std::filesystem::temp_directory_path().string() + '/'
      + ::testing::UnitTest::GetInstance()->current_test_suite()->name() + '_'
      + ::testing::UnitTest::GetInstance()->current_test_info()->name();
  t += ".XXXXXX";

  const std::unique_ptr<char[]> p(new char[t.size() + 1]);
  std::copy_n(t.data(), t.size() + 1, p.get());

  const int fd = mkstemp(p.get());
  EXPECT_NE(-1, fd) << strerror(errno);

  EXPECT_EQ(0, ftruncate(fd, 0)) << strerror(errno);
  EXPECT_EQ(0, close(fd)) << strerror(errno);

  m_log_file = std::string(p.get());
}

bim::server::tests::statistics_log::~statistics_log()
{
  if (::testing::Test::HasFailure())
    {
      printf("Statistics log file is '%s'\n", m_log_file.c_str());
      return;
    }

  if (remove(m_log_file.c_str()) != 0)
    printf("Failed to remove statistics log file '%s': %s\n",
           m_log_file.c_str(), strerror(errno));
}

std::vector<bim::server::tests::statistics_log_line>
bim::server::tests::statistics_log::read_log_file() const
{
  std::ifstream f(m_log_file);
  EXPECT_TRUE(!!f);

  std::vector<statistics_log_line> result;
  std::string line;
  std::tm tm;

  while (std::getline(f, line))
    {
      std::istringstream iss(line);
      statistics_log_line s;

      iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S") >> s.active_sessions
          >> s.players_in_games >> s.games;

      EXPECT_TRUE(!!iss) << "line is '" << line << '\'';

      result.push_back(s);
    }

  return result;
}

const std::string& bim::server::tests::statistics_log::log_file() const
{
  return m_log_file;
}
