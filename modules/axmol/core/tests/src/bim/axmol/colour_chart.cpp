#include <bim/axmol/colour_chart.hpp>

#include <axmol/base/Types.h>

#include <gtest/gtest.h>

TEST(bim_axmol_colour_chart, to_color_3b)
{
  bim::axmol::colour_chart colors;
  ax::Color3B color = colors.to_color_3b("#102030");

  EXPECT_EQ(0x10, color.r);
  EXPECT_EQ(0x20, color.g);
  EXPECT_EQ(0x30, color.b);

  colors.add_alias("yellow", "#f0ff08");
  color = colors.to_color_3b("yellow");

  EXPECT_EQ(0xf0, color.r);
  EXPECT_EQ(0xff, color.g);
  EXPECT_EQ(0x08, color.b);
}
