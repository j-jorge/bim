// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>
#include <bim/server/tests/test_client.hpp>

#include <bim/server/server.hpp>

#include <iscool/log/setup.hpp>
#include <iscool/net/message_channel.hpp>

#include <array>
#include <functional>

namespace bim::server::tests
{
  class client_server_simulator
  {
  public:
    client_server_simulator(std::uint8_t player_count,
                            const bim::server::config& config);
    ~client_server_simulator();

    void authenticate();
    void join_game();
    void tick();
    void tick(std::chrono::nanoseconds d);
    void wait(const std::function<bool()>& ready);

  private:
    const std::uint8_t m_player_count;

    iscool::log::scoped_initializer m_log;
    bim::server::tests::fake_scheduler m_scheduler;

    bim::server::server m_server;
    iscool::net::socket_stream m_socket_stream;
    iscool::net::message_stream m_message_stream;

  public:
    std::array<bim::server::tests::test_client, 4> clients;
  };
}
