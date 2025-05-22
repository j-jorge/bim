// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/server.hpp>

#include <bim/server/config.hpp>

#include <bim/net/message/protocol_version.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/net/message/message.hpp>

bim::server::server::server(const config& config)
  : m_server_stats(bim::server::server_stats(config.stats_dump_delay,
                                             config.log_rotation_interval))
  , m_socket(config.port)
  , m_authentication_service(config, m_socket, m_server_stats)
  , m_game_service(config, m_socket, m_server_stats)
  , m_lobby_service(config, m_socket, m_game_service)
{
  ic_log(iscool::log::nature::info(), "server", "Server is up on port {}.",
         config.port);
  ic_log(iscool::log::nature::info(), "server", "Protocol version is {}.",
         bim::net::protocol_version);

  m_authentication_service.connect_to_message(std::bind(
      &server::dispatch, this, std::placeholders::_1, std::placeholders::_2));
}

void bim::server::server::dispatch(const iscool::net::endpoint& endpoint,
                                   const iscool::net::message& message)
{
  // The message comes from an active session, we can process it.
  if (message.get_channel_id() == 0)
    m_lobby_service.process(endpoint, message);
  else
    m_game_service.process(endpoint, message);
}
