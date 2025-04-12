// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/animation/animation_cache.hpp>

#include <bim/axmol/widget/animation/animation.hpp>

#include <json/value.h>

#include <gtest/gtest.h>

TEST(bim_axmol_widget_animation_cache, load)
{
  Json::Value config;

  config["simple"]["frame_duration.ms"] = 11;
  config["simple"]["frames"][0] = "frame_0";
  config["simple"]["frames"][1] = "frame_1";

  config["flip"]["frame_duration.ms"] = 22;
  config["flip"]["flip.x"] = true;
  config["flip"]["frames"][0] = "frame_0";
  config["flip"]["frames"][1] = "frame_1";

  config["no-loop"]["frame_duration.ms"] = 33;
  config["no-loop"]["loop"] = false;
  config["no-loop"]["frames"][0] = "frame_0";
  config["no-loop"]["frames"][1] = "frame_1";

  config["flip-frame"]["frame_duration.ms"] = 44;
  config["flip-frame"]["flip.x"] = false;
  config["flip-frame"]["frames"][0]["name"] = "frame_0";
  config["flip-frame"]["frames"][0]["flip.x"] = true;
  config["flip-frame"]["frames"][1] = "frame_1";
  config["flip-frame"]["frames"][2]["name"] = "frame_2";
  config["flip-frame"]["frames"][2]["flip.x"] = false;

  bim::axmol::widget::animation_cache cache;
  cache.load(config);

  {
    const bim::axmol::widget::animation& animation = cache.get("simple");

    EXPECT_EQ(22, animation.total_duration.count());

    ASSERT_EQ(2, animation.frames.size());

    EXPECT_EQ(0, animation.frames[0].display_date.count());
    EXPECT_FALSE(animation.frames[0].flip_x);

    EXPECT_EQ(11, animation.frames[1].display_date.count());
    EXPECT_FALSE(animation.frames[1].flip_x);
  }

  {
    const bim::axmol::widget::animation& animation = cache.get("flip");

    EXPECT_EQ(44, animation.total_duration.count());

    ASSERT_EQ(2, animation.frames.size());

    EXPECT_EQ(0, animation.frames[0].display_date.count());
    EXPECT_TRUE(animation.frames[0].flip_x);

    EXPECT_EQ(22, animation.frames[1].display_date.count());
    EXPECT_TRUE(animation.frames[1].flip_x);
  }

  {
    const bim::axmol::widget::animation& animation = cache.get("no-loop");

    EXPECT_EQ(0, animation.total_duration.count());

    ASSERT_EQ(2, animation.frames.size());

    EXPECT_EQ(0, animation.frames[0].display_date.count());
    EXPECT_FALSE(animation.frames[0].flip_x);

    EXPECT_EQ(33, animation.frames[1].display_date.count());
    EXPECT_FALSE(animation.frames[1].flip_x);
  }

  {
    const bim::axmol::widget::animation& animation = cache.get("flip-frame");

    EXPECT_EQ(132, animation.total_duration.count());

    ASSERT_EQ(3, animation.frames.size());

    EXPECT_EQ(0, animation.frames[0].display_date.count());
    EXPECT_TRUE(animation.frames[0].flip_x);

    EXPECT_EQ(44, animation.frames[1].display_date.count());
    EXPECT_FALSE(animation.frames[1].flip_x);

    EXPECT_EQ(88, animation.frames[2].display_date.count());
    EXPECT_FALSE(animation.frames[2].flip_x);
  }
}
