// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>
#include <bim/server/tests/fake_business.hpp>
#include <bim/server/tests/new_test_config.hpp>

#include <bim/net/message/game_id.hpp>
#include <bim/net/message/user_id.hpp>

#include <iscool/json/cast.hpp>
#include <iscool/json/cast_int64.hpp>
#include <iscool/json/cast_string.hpp>

#include <gtest/gtest.h>

class game_business_request_test : public testing::TestWithParam<int>
{
public:
  static constexpr int ticks_for_disconnection = 10;

public:
  game_business_request_test();

  bim::net::game_id start_game();

protected:
  bim::server::tests::client_server_simulator m_simulator;
  bim::server::tests::fake_business m_business;
};

game_business_request_test::game_business_request_test()
  : m_simulator(
        GetParam(),
        []()
          {
            bim::server::config config = bim::server::tests::new_test_config();

            config.business_url = "biz/";
            config.game_service_disconnection_inactivity_delay =
                std::chrono::seconds(50);

            config.game_service_disconnection_earliness_threshold_in_ticks =
                std::numeric_limits<int>::max();
            config.game_service_disconnection_lateness_threshold_in_ticks =
                ticks_for_disconnection;
            config.game_service_disconnection_inactivity_delay =
                std::chrono::hours(10);

            return config;
          }())
{}

bim::net::game_id game_business_request_test::start_game()
{
  const bim::net::game_id game_id = m_business.next_game_id;

  m_simulator.authenticate_with_business_token();
  m_simulator.join_game();

  const int player_count = GetParam();
  const Json::Value& players = m_business.last_game_started_request["players"];
  EXPECT_EQ(player_count, players.size());

  for (int i = 0; i != player_count; ++i)
    {
      const bim::server::tests::test_client& client = m_simulator.clients[i];
      EXPECT_EQ(client.user_id, iscool::json::cast<bim::net::user_id>(
                                    players[client.player_index]))
          << "i=" << i;
    }

  EXPECT_EQ(game_id, iscool::json::cast<bim::net::game_id>(
                         m_business.last_game_started_response["game_id"]));

  return game_id;
}

TEST_P(game_business_request_test, winner)
{
  const bim::net::game_id game_id = start_game();

  const int client_count = GetParam();
  const int surviving_client = client_count / 2;
  m_simulator.drop_bombs(~(1 << surviving_client));
  m_simulator.wait_game_over();

  EXPECT_EQ(game_id, iscool::json::cast<bim::net::game_id>(
                         m_business.last_game_over_request["game_id"]));

  bool got_victory = false;
  bool got_defeated = false;

  for (int i = 0; i != client_count; ++i)
    {
      const bim::server::tests::test_client& client = m_simulator.clients[i];

      EXPECT_EQ(client.user_id,
                iscool::json::cast<bim::net::user_id>(
                    m_business.last_game_over_request["players"]
                                                     [client.player_index]))
          << "i=" << i;

      const std::string outcome = iscool::json::cast<std::string>(
          m_business.last_game_over_request["outcome"][client.player_index]);

      if (i == surviving_client)
        {
          EXPECT_EQ("victory", outcome) << "i=" << i;
          got_victory = true;
        }
      else
        {
          EXPECT_EQ("defeated", outcome) << "i=" << i;
          got_defeated = true;
        }
    }

  EXPECT_TRUE(got_victory);
  EXPECT_TRUE(got_defeated);
}

TEST_P(game_business_request_test, everybody_loses)
{
  const bim::net::game_id game_id = start_game();

  const int client_count = GetParam();
  const int defeated_clients = (1 << client_count) - 1;
  m_simulator.drop_bombs(defeated_clients);
  m_simulator.wait_game_over();

  EXPECT_EQ(game_id, iscool::json::cast<bim::net::game_id>(
                         m_business.last_game_over_request["game_id"]));

  for (int i = 0; i != client_count; ++i)
    {
      const bim::server::tests::test_client& client = m_simulator.clients[i];

      EXPECT_EQ(client.user_id,
                iscool::json::cast<bim::net::user_id>(
                    m_business.last_game_over_request["players"]
                                                     [client.player_index]))
          << "i=" << i;

      const std::string outcome = iscool::json::cast<std::string>(
          m_business.last_game_over_request["outcome"][client.player_index]);

      EXPECT_EQ("defeated", outcome) << "i=" << i;
    }
}

TEST_P(game_business_request_test, draw)
{
  const bim::net::game_id game_id = start_game();

  const int client_count = GetParam();
  // Keep two clients alive.
  const int defeated_clients = (1 << (client_count - 2)) - 1;
  m_simulator.drop_bombs(defeated_clients);
  m_simulator.wait_game_over();

  EXPECT_EQ(game_id, iscool::json::cast<bim::net::game_id>(
                         m_business.last_game_over_request["game_id"]));

  bool got_draw = false;
  bool got_defeated = false;

  for (int i = 0; i != client_count; ++i)
    {
      const bim::server::tests::test_client& client = m_simulator.clients[i];

      EXPECT_EQ(client.user_id,
                iscool::json::cast<bim::net::user_id>(
                    m_business.last_game_over_request["players"]
                                                     [client.player_index]))
          << "i=" << i;

      const std::string outcome = iscool::json::cast<std::string>(
          m_business.last_game_over_request["outcome"][client.player_index]);

      if ((1 << i) & defeated_clients)
        {
          EXPECT_EQ("defeated", outcome) << "i=" << i;
          got_defeated = true;
        }
      else
        {
          EXPECT_EQ("draw", outcome) << "i=" << i;
          got_draw = true;
        }
    }

  EXPECT_TRUE(got_draw);

  if (client_count > 2)
    {
      EXPECT_TRUE(got_defeated);
    }
}

TEST_P(game_business_request_test, kicked)
{
  const bim::net::game_id game_id = start_game();

  const int client_count = GetParam();
  // Keep two clients alive.
  const int surviving_client = client_count / 2;
  const int kicked_client = surviving_client - 1;
  m_simulator.clients[kicked_client].leave_game();

  m_simulator.tick(ticks_for_disconnection);

  m_simulator.drop_bombs(~(1 << surviving_client));
  m_simulator.wait_game_over();

  EXPECT_EQ(game_id, iscool::json::cast<bim::net::game_id>(
                         m_business.last_game_over_request["game_id"]));

  bool got_victory = false;
  bool got_kicked = false;
  bool got_defeated = false;

  for (int i = 0; i != client_count; ++i)
    {
      const bim::server::tests::test_client& client = m_simulator.clients[i];

      EXPECT_EQ(client.user_id,
                iscool::json::cast<bim::net::user_id>(
                    m_business.last_game_over_request["players"]
                                                     [client.player_index]))
          << "i=" << i;

      const std::string outcome = iscool::json::cast<std::string>(
          m_business.last_game_over_request["outcome"][client.player_index]);

      if (i == surviving_client)
        {
          EXPECT_EQ("victory", outcome) << "i=" << i;
          got_victory = true;
        }
      else if (i == kicked_client)
        {
          EXPECT_EQ("kicked", outcome) << "i=" << i;
          got_kicked = true;
        }
      else
        {
          got_defeated = true;
          EXPECT_EQ("defeated", outcome) << "i=" << i;
        }
    }

  EXPECT_TRUE(got_victory);
  EXPECT_TRUE(got_kicked);

  if (client_count > 2)
    {
      EXPECT_TRUE(got_defeated);
    }
}

INSTANTIATE_TEST_SUITE_P(game_business_request_test_instance,
                         game_business_request_test, testing::Values(2, 3, 4));
