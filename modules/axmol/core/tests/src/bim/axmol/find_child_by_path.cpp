// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/find_child_by_path.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <axmol/2d/Node.h>

#include <gtest/gtest.h>

TEST(bim_axmol_find_child_by_path, general_case)
{
  const bim::axmol::ref_ptr<ax::Node> n[5] = {
    ax::Node::create(), ax::Node::create(), ax::Node::create(),
    ax::Node::create(), ax::Node::create()
  };

  n[0]->setName("root");
  n[1]->setName("one");
  n[2]->setName("two");
  n[3]->setName("three");
  n[4]->setName("four");

  for (int i = 0; i != 4; ++i)
    n[i]->addChild(n[i + 1].get());

  EXPECT_EQ(n[1].get(), bim::axmol::find_child_by_path(*n[0], "one"));
  EXPECT_EQ(n[1].get(), bim::axmol::find_child_by_path(*n[0], "one/"));
  EXPECT_EQ(nullptr, bim::axmol::find_child_by_path(*n[0], "nope"));
  EXPECT_EQ(nullptr, bim::axmol::find_child_by_path(*n[0], "one/nope/two"));
  EXPECT_EQ(nullptr, bim::axmol::find_child_by_path(*n[0], "one/nope/three"));

  EXPECT_EQ(n[2].get(), bim::axmol::find_child_by_path(*n[0], "one/two"));
  EXPECT_EQ(n[2].get(), bim::axmol::find_child_by_path(*n[0], "one/two/"));
  EXPECT_EQ(nullptr, bim::axmol::find_child_by_path(*n[0], "one/nope"));

  EXPECT_EQ(n[3].get(),
            bim::axmol::find_child_by_path(*n[0], "one/two/three"));
  EXPECT_EQ(n[3].get(),
            bim::axmol::find_child_by_path(*n[0], "one/two/three/"));
  EXPECT_EQ(nullptr, bim::axmol::find_child_by_path(*n[0], "one/two/nope"));

  EXPECT_EQ(n[4].get(),
            bim::axmol::find_child_by_path(*n[0], "one/two/three/four"));
  EXPECT_EQ(n[4].get(),
            bim::axmol::find_child_by_path(*n[0], "one/two/three/four/"));
  EXPECT_EQ(nullptr,
            bim::axmol::find_child_by_path(*n[0], "one/two/three/nope"));
}
