// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/displayed.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <axmol/2d/Node.h>

#include <gtest/gtest.h>

TEST(bim_axmol_displayed, root)
{
  const bim::axmol::ref_ptr<ax::Node> n = ax::Node::create();

  EXPECT_FALSE(bim::axmol::displayed(*n));

  n->onEnter();
  EXPECT_TRUE(bim::axmol::displayed(*n));

  n->setVisible(false);
  EXPECT_FALSE(bim::axmol::displayed(*n));

  n->setVisible(true);
  n->onExit();
  EXPECT_FALSE(bim::axmol::displayed(*n));
}

TEST(bim_axmol_displayed, hierarchy)
{
  const bim::axmol::ref_ptr<ax::Node> n = ax::Node::create();
  const bim::axmol::ref_ptr<ax::Node> parent = ax::Node::create();
  parent->addChild(n.get());

  const bim::axmol::ref_ptr<ax::Node> grandparent = ax::Node::create();
  grandparent->addChild(parent.get());

  EXPECT_FALSE(bim::axmol::displayed(*n));

  grandparent->onEnter();
  EXPECT_TRUE(bim::axmol::displayed(*n));

  // When the node itself is hidden.
  n->setVisible(false);
  EXPECT_FALSE(bim::axmol::displayed(*n));

  // When the parent is hidden.
  n->setVisible(true);
  EXPECT_TRUE(bim::axmol::displayed(*n));

  parent->setVisible(false);
  EXPECT_FALSE(bim::axmol::displayed(*n));

  // When the grand parent is hidden.
  parent->setVisible(true);
  EXPECT_TRUE(bim::axmol::displayed(*n));

  parent->setVisible(true);
  EXPECT_TRUE(bim::axmol::displayed(*n));

  grandparent->setVisible(false);
  EXPECT_FALSE(bim::axmol::displayed(*n));

  // When the grandparent is not running.
  grandparent->setVisible(true);
  grandparent->onExit();
  EXPECT_FALSE(bim::axmol::displayed(*n));
}
