// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/contest_timeline_service.hpp>

#include <bim/server/config.hpp>

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
  std::ostringstream file_name;
  const std::time_t t = std::time(nullptr);

  file_name << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S") << '_'
            << getpid() << '_' << this << '_' << std::setfill('0')
            << std::setw(10) << channel << ".bim";

  return bim::game::contest_timeline_writer(
      m_directory / std::move(file_name).str(), contest);
}
