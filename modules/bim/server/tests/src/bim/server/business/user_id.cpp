// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/business/user_id.hpp>

#include <json/value.h>

#include <gtest/gtest.h>

TEST(bim_server_business_user_id, from_json)
{
  Json::Value json;
  json["tokens"][0] = 1;
  json["tokens"][1] = 2;
  json["tokens"][2] = 3;
  json["tokens"][3] = 4;
  json["tokens"][4] = 5;

  json["user_ids"][0] = 10;
  json["user_ids"][1] = Json::nullValue;
  json["user_ids"][2] = 12;
  json["user_ids"][3] = Json::nullValue;
  json["user_ids"][4] = 14;

  bim::server::business::user_id_response r;
  from_json(r, json);

  ASSERT_EQ(2, r.rejected.size());
  ASSERT_EQ(3, r.accepted.size());
  ASSERT_EQ(3, r.user_id.size());

  EXPECT_EQ(2, r.rejected[0]);
  EXPECT_EQ(4, r.rejected[1]);

  EXPECT_EQ(1, r.accepted[0]);
  EXPECT_EQ(3, r.accepted[1]);
  EXPECT_EQ(5, r.accepted[2]);

  EXPECT_EQ(10, r.user_id[0]);
  EXPECT_EQ(12, r.user_id[1]);
  EXPECT_EQ(14, r.user_id[2]);
}
