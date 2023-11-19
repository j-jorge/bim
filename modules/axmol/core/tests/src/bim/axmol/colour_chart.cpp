#include <bim/axmol/colour_chart.hpp>

#include <axmol/base/Types.h>

#include <gtest/gtest.h>

TEST(bim_axmol_colour_chart, to_color_3b)
{
  bim::axmol::colour_chart colors;
  ax::Color4B color = colors.to_color_4b("#102030");

  EXPECT_EQ(0x10, color.r);
  EXPECT_EQ(0x20, color.g);
  EXPECT_EQ(0x30, color.b);
  EXPECT_EQ(0xff, color.a);

  colors.add_alias("yellow", "#f0ff08");
  color = colors.to_color_4b("yellow");

  EXPECT_EQ(0xf0, color.r);
  EXPECT_EQ(0xff, color.g);
  EXPECT_EQ(0x08, color.b);
  EXPECT_EQ(0xff, color.a);
}

TEST(bim_axmol_colour_chart, to_color_4b)
{
  bim::axmol::colour_chart colors;
  ax::Color4B color = colors.to_color_4b("#102030c0");

  EXPECT_EQ(0x10, color.r);
  EXPECT_EQ(0x20, color.g);
  EXPECT_EQ(0x30, color.b);
  EXPECT_EQ(0xc0, color.a);

  colors.add_alias("yellow", "#f0ff087f");
  color = colors.to_color_4b("yellow");

  EXPECT_EQ(0xf0, color.r);
  EXPECT_EQ(0xff, color.g);
  EXPECT_EQ(0x08, color.b);
  EXPECT_EQ(0x7f, color.a);
}
