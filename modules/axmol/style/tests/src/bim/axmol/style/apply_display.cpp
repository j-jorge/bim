#include <bim/axmol/style/apply_display.hpp>

#include <bim/axmol/style/display_properties.hpp>
#include <bim/axmol/style/display_property_flags.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <axmol/2d/Node.h>

#include <gtest/gtest.h>

TEST(bim_axmol_style_apply_display, none)
{
  bim::axmol::style::display_properties display{};

  display.color = ax::Color3B::MAGENTA;
  display.anchor_point_x = 1;
  display.anchor_point_y = -1;
  display.rotation = 10;
  display.opacity = 0.2;
  display.z_order = 30;
  display.cascade_opacity = false;
  display.cascade_color = false;
  display.visible = false;

  const bim::axmol::ref_ptr<ax::Node> node(ax::Node::create());

  node->setColor(ax::Color3B::YELLOW);
  node->setAnchorPoint(ax::Vec2(0.24, 1.7));
  node->setRotation(123);
  node->setOpacity(92);
  node->setLocalZOrder(11);
  node->setCascadeOpacityEnabled(true);
  node->setCascadeColorEnabled(true);
  node->setVisible(true);

  bim::axmol::style::apply_display(display, *node);

  EXPECT_EQ(ax::Color3B::YELLOW, node->getColor());
  EXPECT_FLOAT_EQ(0.24, node->getAnchorPoint().x);
  EXPECT_FLOAT_EQ(1.7, node->getAnchorPoint().y);
  EXPECT_FLOAT_EQ(123, node->getRotation());
  EXPECT_EQ(92, node->getOpacity());
  EXPECT_EQ(11, node->getLocalZOrder());
  EXPECT_TRUE(node->isCascadeOpacityEnabled());
  EXPECT_TRUE(node->isCascadeColorEnabled());
  EXPECT_TRUE(node->isVisible());
}

TEST(bim_axmol_style_apply_display, all)
{
  bim::axmol::style::display_properties display{};

  display.flags = bim::axmol::style::display_property_flags::color
                  | bim::axmol::style::display_property_flags::anchor_point_x
                  | bim::axmol::style::display_property_flags::anchor_point_y
                  | bim::axmol::style::display_property_flags::rotation
                  | bim::axmol::style::display_property_flags::opacity
                  | bim::axmol::style::display_property_flags::z_order
                  | bim::axmol::style::display_property_flags::cascade_opacity
                  | bim::axmol::style::display_property_flags::cascade_color
                  | bim::axmol::style::display_property_flags::visible;

  display.color = ax::Color3B::MAGENTA;
  display.anchor_point_x = 1;
  display.anchor_point_y = -1;
  display.rotation = 10;
  display.opacity = 0.2;
  display.z_order = 30;
  display.cascade_opacity = false;
  display.cascade_color = false;
  display.visible = false;

  const bim::axmol::ref_ptr<ax::Node> node(ax::Node::create());

  node->setColor(ax::Color3B::YELLOW);
  node->setAnchorPoint(ax::Vec2(0.24, 1.7));
  node->setRotation(123);
  node->setOpacity(92);
  node->setLocalZOrder(11);
  node->setCascadeOpacityEnabled(true);
  node->setVisible(true);

  bim::axmol::style::apply_display(display, *node);

  EXPECT_EQ(ax::Color3B::MAGENTA, node->getColor());
  EXPECT_FLOAT_EQ(1, node->getAnchorPoint().x);
  EXPECT_FLOAT_EQ(-1, node->getAnchorPoint().y);
  EXPECT_FLOAT_EQ(10, node->getRotation());
  EXPECT_EQ(0.2 * 255, node->getOpacity());
  EXPECT_EQ(30, node->getLocalZOrder());
  EXPECT_FALSE(node->isCascadeOpacityEnabled());
  EXPECT_FALSE(node->isCascadeColorEnabled());
  EXPECT_FALSE(node->isVisible());
}
