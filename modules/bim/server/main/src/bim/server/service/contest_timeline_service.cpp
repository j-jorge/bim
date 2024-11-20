// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/contest_timeline_service.hpp>

#include <bim/server/config.hpp>

#include <cstring>
#include <iomanip>
#include <sstream>

#include <unistd.h>

bim::server::contest_timeline_service::contest_timeline_service(
    const config& config)
  : m_directory(config.contest_timeline_folder)
{}

bim::game::contest_timeline_writer bim::server::contest_timeline_service::open(
    iscool::net::channel_id channel,
    const bim::game::contest_fingerprint& contest)
{
  // We are going to try to build a unique name for the file, even if multiple
  // instances are writing the same game channel. We use the process id and the
  // memory address of this instance, but even this is not enough in the case
  // of multiple runs, so we will add an additional suffix below in an attempt
  // to discriminate.

  const std::time_t t = std::time(nullptr);
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S") << '_' << getpid()
      << '_' << this << '_' << std::setfill('0') << std::setw(10) << channel;
  const std::string base_file_name(std::move(oss).str());
  std::string file_path;
  file_path.reserve(m_directory.size() + sizeof('/') + base_file_name.size()
                    + std::strlen("_x.bim"));

  std::FILE* file = nullptr;

  for (int i = 0; (i != 10) && !file; ++i)
    {
      file_path.clear();
      file_path += m_directory;
      file_path += '/';
      file_path += base_file_name;

      if (i != 0)
        {
          file_path += '_';
          file_path += std::to_string(i);
        }

      file_path += ".bim";
      file = std::fopen(file_path.c_str(), "wx");
    }

  if (!file)
    return {};

  return bim::game::contest_timeline_writer(file, contest);
}
