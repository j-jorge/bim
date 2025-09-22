// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/hello_ok.hpp>

#include <iscool/net/byte_array.hpp>
#include <iscool/net/byte_array_reader.hpp>

#include <limits>

namespace
{
  // The idea is to have a flexible list of records in the message such that it
  // can be read even by apps not using the latest protocol
  // version. Consequently, do not change the order of the values in this enum.
  enum class hello_record_id : std::uint8_t
  {
    games_now,
    games_last_hour,
    games_last_day,
    games_last_month,
    sessions_now,
    sessions_last_hour,
    sessions_last_day,
    sessions_last_month,
    name
  };
}

bim::net::hello_ok::hello_ok()
  : games_now(0)
  , games_last_hour(0)
  , games_last_day(0)
  , games_last_month(0)
  , sessions_now(0)
  , sessions_last_hour(0)
  , sessions_last_day(0)
  , sessions_last_month(0)
{}

bim::net::hello_ok::hello_ok(const iscool::net::byte_array& raw_content)
{
  iscool::net::byte_array_reader reader(raw_content);

  reader >> request_token >> version;

  while (reader.has_next())
    {
      hello_record_id id;
      std::uint8_t value_size;
      reader >> id >> value_size;

      switch (id)
        {
        case hello_record_id::games_now:
          reader >> games_now;
          break;
        case hello_record_id::games_last_hour:
          reader >> games_last_hour;
          break;
        case hello_record_id::games_last_day:
          reader >> games_last_day;
          break;
        case hello_record_id::games_last_month:
          reader >> games_last_month;
          break;
        case hello_record_id::sessions_now:
          reader >> sessions_now;
          break;
        case hello_record_id::sessions_last_hour:
          reader >> sessions_last_hour;
          break;
        case hello_record_id::sessions_last_day:
          reader >> sessions_last_day;
          break;
        case hello_record_id::sessions_last_month:
          reader >> sessions_last_month;
          break;
        case hello_record_id::name:
          {
            const std::span<const std::uint8_t> raw_name =
                reader.raw(value_size);

            name.clear();
            name.reserve(value_size);
            name.insert(name.end(), raw_name.begin(), raw_name.end());
            break;
          }
        default:
          reader.raw(value_size);
        }
    }
}

void bim::net::hello_ok::build_message(iscool::net::message& message) const
{
  message.reset(get_type());

  iscool::net::byte_array& content = message.get_content();
  content << request_token << version;

  content << hello_record_id::games_now << (std::uint8_t)(sizeof(games_now))
          << games_now;

  content << hello_record_id::games_last_hour
          << (std::uint8_t)(sizeof(games_last_hour)) << games_last_hour;

  content << hello_record_id::games_last_day
          << (std::uint8_t)(sizeof(games_last_day)) << games_last_day;

  content << hello_record_id::games_last_month
          << (std::uint8_t)(sizeof(games_last_month)) << games_last_month;

  content << hello_record_id::sessions_now
          << (std::uint8_t)(sizeof(sessions_now)) << sessions_now;

  content << hello_record_id::sessions_last_hour
          << (std::uint8_t)(sizeof(sessions_last_hour)) << sessions_last_hour;

  content << hello_record_id::sessions_last_day
          << (std::uint8_t)(sizeof(sessions_last_day)) << sessions_last_day;

  content << hello_record_id::sessions_last_month
          << (std::uint8_t)(sizeof(sessions_last_month))
          << sessions_last_month;

  if (name.size() > std::numeric_limits<std::uint8_t>::max())
    throw std::runtime_error("Server name is too long.");

  content << hello_record_id::name << (std::uint8_t)name.size();

  for (char c : name)
    content << (std::uint8_t)c;
}
