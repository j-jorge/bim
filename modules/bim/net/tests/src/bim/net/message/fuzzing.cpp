// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/accept_named_game.hpp>
#include <bim/net/message/accept_random_game.hpp>
#include <bim/net/message/acknowledge_keep_alive.hpp>
#include <bim/net/message/authentication.hpp>
#include <bim/net/message/authentication_ko.hpp>
#include <bim/net/message/authentication_ok.hpp>
#include <bim/net/message/game_on_hold.hpp>
#include <bim/net/message/game_over.hpp>
#include <bim/net/message/game_update_from_client.hpp>
#include <bim/net/message/game_update_from_server.hpp>
#include <bim/net/message/hello.hpp>
#include <bim/net/message/hello_ok.hpp>
#include <bim/net/message/keep_alive.hpp>
#include <bim/net/message/launch_game.hpp>
#include <bim/net/message/new_named_game_request.hpp>
#include <bim/net/message/new_random_game_request.hpp>
#include <bim/net/message/player_action_serialization.hpp>
#include <bim/net/message/ready.hpp>
#include <bim/net/message/start.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <cstdlib>
#include <limits>
#include <random>

#include <gtest/gtest.h>

template <typename M>
class fuzzed_message_deserialization_test : public testing::Test
{
public:
  fuzzed_message_deserialization_test()
  {
    const char* const seed = std::getenv("BIM_TEST_SEED");

    if (seed)
      {
        m_first_seed = std::atoi(seed);
        m_loops = 1;
      }
    else
      {
        m_seed_source = std::mt19937(std::random_device()());
        m_first_seed = random();
        m_loops = 100;
      }
  }

protected:
  std::mt19937 m_seed_source;
  std::uint32_t m_first_seed;
  int m_loops;
};

using all_message_types = testing::Types<
    bim::net::accept_named_game, bim::net::accept_random_game,
    bim::net::acknowledge_keep_alive, bim::net::authentication,
    bim::net::authentication_ko, bim::net::authentication_ok,
    bim::net::game_on_hold, bim::net::game_over,
    bim::net::game_update_from_client, bim::net::game_update_from_server,
    bim::net::hello, bim::net::hello_ok, bim::net::keep_alive,
    bim::net::launch_game, bim::net::new_named_game_request,
    bim::net::new_random_game_request, bim::net::ready, bim::net::start>;

TYPED_TEST_SUITE(fuzzed_message_deserialization_test, all_message_types);

static iscool::net::byte_array random_buffer(std::uint32_t seed)
{
  std::mt19937 random(seed);
  // std::uniform_int_distribution<std::uint8_t> is UB so we will use ints and
  // cast below.
  std::uniform_int_distribution<int> ints(
      0, std::numeric_limits<std::uint8_t>::max());

  iscool::net::byte_array buffer;
  const std::size_t size = ints(random);
  buffer.reserve(size);

  for (std::size_t i = 0; i != size; ++i)
    buffer.append((std::uint8_t)ints(random));

  return buffer;
}

template <typename M>
static void test_deserialization(std::uint32_t seed)
{
  const iscool::net::message message(M::get_type(), random_buffer(seed));
  bim::net::try_deserialize_message<M>(message);
}

TYPED_TEST(fuzzed_message_deserialization_test, fully_random)
{
  std::uint32_t seed = this->m_first_seed;

  for (int i = 0; i != this->m_loops; ++i)
    {
      ASSERT_EXIT(test_deserialization<TypeParam>(seed);
                  std::exit(0), testing::ExitedWithCode(0), "")
          << "seed=" << seed;
      seed = this->m_seed_source();
    }
}
