#include <bim/axmol/widget/factory/node.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/dynamic_factory.hpp>

#include <bim/axmol/style/cache.hpp>

#include <bim/axmol/colour_chart.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/Node.h>

#include <gtest/gtest.h>

TEST(bim_axmol_widget_factory_node, factory)
{
  bim::axmol::colour_chart colors;
  bim::axmol::style::cache style_cache(colors);
  bim::axmol::widget::dynamic_factory dynamic_factory;
  bim::axmol::widget::context context(colors, style_cache, dynamic_factory);

  iscool::style::declaration style;
  style.set_number("anchor-point.x", 0.1);
  style.set_number("anchor-point.y", 0.2);
  style.set_string("color", "#ff10c0");
  style.set_number("rotation", 0.3);
  style.set_number("opacity", 0.4);
  style.set_boolean("cascade-opacity", true);
  style.set_boolean("visible", false);
  style.set_number("z_order", 50);

  const bim::axmol::ref_ptr<ax::Node> node =
      bim::axmol::widget::factory<ax::Node>::create(context, style);

  ASSERT_NE(nullptr, node);
  EXPECT_FLOAT_EQ(0.1, node->getAnchorPoint().x);
  EXPECT_FLOAT_EQ(0.2, node->getAnchorPoint().y);
  EXPECT_EQ(0xff, node->getColor().r);
  EXPECT_EQ(0x10, node->getColor().g);
  EXPECT_EQ(0xc0, node->getColor().b);
  EXPECT_FLOAT_EQ(0.3, node->getRotation());
  EXPECT_EQ(0.4 * 255, node->getOpacity());
  EXPECT_TRUE(node->isCascadeOpacityEnabled());
  EXPECT_TRUE(node->isVisible());
  EXPECT_EQ(50, node->getLocalZOrder());
}
