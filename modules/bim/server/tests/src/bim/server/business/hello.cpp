// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/business/hello.hpp>

#include <json/value.h>

#include <gtest/gtest.h>

TEST(bim_server_business_hello, from_json)
{
  Json::Value json;
  json["callback_delay_seconds"] = 123;

  bim::server::business::hello_response r;
  from_json(r, json);

  EXPECT_EQ(123, r.callback_delay.count());
}
