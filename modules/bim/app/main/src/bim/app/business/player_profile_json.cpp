// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/player_profile_json.hpp>

#include <bim/app/business/player_profile.hpp>

#include <bim/game/feature_flags_string.hpp>

#include <iscool/json/cast.hpp>
#include <iscool/json/cast_bool.hpp>
#include <iscool/json/cast_int64.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/json/cast_uint64.hpp>

#include <json/value.h>

bool bim::app::from_json(player_profile& p, const Json::Value& json)
{
  p.nickname = iscool::json::member_cast<std::string>(json, "nickname");
  p.coins = iscool::json::member_cast<std::int64_t>(json, "coins");
  p.user_id = iscool::json::member_cast<bim::net::user_id>(json, "user_id");

  p.slot_availability.fill(false);
  p.slot_feature.fill({});

  const Json::Value& feature_slots = json["feature_slots"];

  if (!feature_slots.isArray())
    return false;

  for (Json::ArrayIndex i = 0, n = feature_slots.size(); i != n; ++i)
    {
      const Json::Value& feature_slot = feature_slots[i];
      const Json::Value& feature = feature_slot["feature"];
      const std::size_t slot_index =
          iscool::json::member_cast<std::uint64_t>(feature_slot, "slot_index");

      if (slot_index >= p.slot_availability.size())
        return false;

      if (!feature.isNull())
        {
          const std::optional<bim::game::feature_flags> f =
              bim::game::from_simple_string(
                  iscool::json::cast<std::string>(feature));

          if (!f)
            return false;

          p.slot_feature[slot_index] = *f;
        }

      p.slot_availability[slot_index] = true;
    }

  const Json::Value& available_features = json["available_features"];

  if (!available_features.isArray())
    return false;

  p.available_features = {};

  for (Json::ArrayIndex i = 0, n = available_features.size(); i != n; ++i)
    {
      const std::optional<bim::game::feature_flags> f =
          bim::game::from_simple_string(
              iscool::json::cast<std::string>(available_features[i]));

      if (!f)
        return false;

      p.available_features |= *f;
    }

  const Json::Value* const arena_stats = json.find("arena_stats");

  if (!arena_stats)
    return false;

  p.arena_victories =
      iscool::json::member_cast<std::int64_t>(*arena_stats, "victories");
  p.arena_defeats =
      iscool::json::member_cast<std::int64_t>(*arena_stats, "defeats");
  p.arena_draws =
      iscool::json::member_cast<std::int64_t>(*arena_stats, "draws");

  return true;
}
