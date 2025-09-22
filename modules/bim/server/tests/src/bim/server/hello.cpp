// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/test_client.hpp>

#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/tests/new_test_config.hpp>

#include <bim/server/server.hpp>

#include <bim/net/message/hello.hpp>
#include <bim/net/message/hello_ok.hpp>
#include <bim/net/message/protocol_version.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <iscool/log/setup.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/schedule/manual_scheduler.hpp>
#include <iscool/schedule/setup.hpp>
#include <iscool/signals/scoped_connection.hpp>
#include <iscool/time/setup.hpp>

#include <optional>

#include <gtest/gtest.h>

class hello_test : public testing::Test
{
public:
  hello_test();

protected:
  void test_full_exchange(const bim::net::hello& message);

private:
  void interpret_received_message(bim::net::client_token token,
                                  const iscool::net::message& message);

protected:
  iscool::log::scoped_initializer m_log;
  bim::server::tests::fake_scheduler m_scheduler;

  const bim::server::config m_config;
  bim::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;
  iscool::net::message_channel m_message_channel;

  std::optional<bim::net::hello_ok> m_answer_ok;
};

hello_test::hello_test()
  : m_config(
        []() -> bim::server::config
        {
          bim::server::config config = bim::server::tests::new_test_config();
          config.name = "hello_test";

          return config;
        }())
  , m_server(m_config)
  , m_socket_stream("localhost:" + std::to_string(m_config.port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_message_channel(m_message_stream, 0, 0)
{}

/// Send a message until we get an answer.
void hello_test::test_full_exchange(const bim::net::hello& message)
{
  const iscool::signals::scoped_connection connection =
      m_message_channel.connect_to_message(
          std::bind(&hello_test::interpret_received_message, this,
                    message.get_request_token(), std::placeholders::_2));

  iscool::net::message m;
  message.build_message(m);

  for (int i = 0; (i != 10) && !m_answer_ok; ++i)
    {
      m_message_channel.send(m);
      m_scheduler.tick(std::chrono::seconds(1));
    }
}

void hello_test::interpret_received_message(
    bim::net::client_token token, const iscool::net::message& message)
{
  if (message.get_type() == bim::net::message_type::hello_ok)
    {
      std::optional<bim::net::hello_ok> answer =
          bim::net::try_deserialize_message<bim::net::hello_ok>(message);

      if (answer && (answer->request_token == token))
        m_answer_ok = std::move(*answer);
    }
}

TEST_F(hello_test, ok)
{
  const bim::net::client_token token = 1;
  test_full_exchange(bim::net::hello(token));

  ASSERT_TRUE(!!m_answer_ok);

  EXPECT_EQ(token, m_answer_ok->request_token);
  EXPECT_EQ(bim::net::protocol_version, m_answer_ok->version);

  EXPECT_EQ(0, m_answer_ok->games_now);
  EXPECT_EQ(0, m_answer_ok->games_last_hour);
  EXPECT_EQ(0, m_answer_ok->games_last_day);
  EXPECT_EQ(0, m_answer_ok->games_last_month);

  EXPECT_EQ(0, m_answer_ok->sessions_now);
  EXPECT_EQ(0, m_answer_ok->sessions_last_hour);
  EXPECT_EQ(0, m_answer_ok->sessions_last_day);
  EXPECT_EQ(0, m_answer_ok->sessions_last_month);

  EXPECT_EQ("hello_test", m_answer_ok->name);
}

TEST_F(hello_test, stats)
{
  bim::server::tests::test_client clients[5] = {
    bim::server::tests::test_client(m_scheduler, m_message_stream),
    bim::server::tests::test_client(m_scheduler, m_message_stream),
    bim::server::tests::test_client(m_scheduler, m_message_stream),
    bim::server::tests::test_client(m_scheduler, m_message_stream),
    bim::server::tests::test_client(m_scheduler, m_message_stream)
  };

  for (bim::server::tests::test_client& client : clients)
    client.authenticate();

  for (int i = 0; i != 3; ++i)
    clients[i].new_game();

  int started_count = 0;
  for (int i = 0; (i != 500) && (started_count != 3); ++i)
    {
      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::milliseconds(20));
      started_count = clients[0].is_in_game() + clients[1].is_in_game()
                      + clients[2].is_in_game();
    }

  ASSERT_EQ(3, started_count);

  const bim::net::client_token token = 2;
  test_full_exchange(bim::net::hello(token));

  ASSERT_TRUE(!!m_answer_ok);

  EXPECT_EQ(token, m_answer_ok->request_token);
  EXPECT_EQ(bim::net::protocol_version, m_answer_ok->version);

  EXPECT_EQ(1, m_answer_ok->games_now);
  EXPECT_EQ(1, m_answer_ok->games_last_hour);
  EXPECT_EQ(1, m_answer_ok->games_last_day);
  EXPECT_EQ(1, m_answer_ok->games_last_month);

  EXPECT_EQ(5, m_answer_ok->sessions_now);
  EXPECT_EQ(5, m_answer_ok->sessions_last_hour);
  EXPECT_EQ(5, m_answer_ok->sessions_last_day);
  EXPECT_EQ(5, m_answer_ok->sessions_last_month);

  EXPECT_EQ("hello_test", m_answer_ok->name);
}
