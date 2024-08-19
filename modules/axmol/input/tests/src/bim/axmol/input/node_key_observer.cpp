// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/key_event_view.hpp>
#include <bim/axmol/input/key_observer.hpp>
#include <bim/axmol/input/node.hpp>
#include <bim/axmol/input/node_reference.hpp>

#include <gtest/gtest.h>

namespace
{
  class key_observer_mockup : public bim::axmol::input::key_observer
  {
  public:
    key_observer_mockup(std::string name, std::vector<std::string>& calls);

  private:
    void do_pressed(const bim::axmol::input::key_event_view& keys) override;
    void do_released(const bim::axmol::input::key_event_view& keys) override;

  public:
    std::size_t m_pressed_call_count;

  private:
    const std::string m_name;
    std::vector<std::string>& m_calls;
  };
}

key_observer_mockup::key_observer_mockup(std::string name,
                                         std::vector<std::string>& calls)
  : m_pressed_call_count(0)
  , m_name(std::move(name))
  , m_calls(calls)
{}

void key_observer_mockup::do_pressed(
    const bim::axmol::input::key_event_view& keys)
{
  m_calls.push_back(m_name);
  ++m_pressed_call_count;
}

void key_observer_mockup::do_released(
    const bim::axmol::input::key_event_view& keys)
{
  assert(false);
}

class node_key_observer_test : public ::testing::Test
{
protected:
  std::vector<std::string> m_calls;
};

static void simulate_key_press(bim::axmol::input::node& node)
{
  bim::axmol::input::key_event events(ax::EventKeyboard::KeyCode::KEY_BACK);
  bim::axmol::input::key_event_view keys(std::span(&events, 1));
  node.key_pressed(keys);
}

TEST_F(node_key_observer_test, root)
{
  std::shared_ptr<key_observer_mockup> observer(
      std::make_shared<key_observer_mockup>("root", m_calls));
  bim::axmol::input::node node(observer);

  simulate_key_press(node);

  EXPECT_EQ(1, observer->m_pressed_call_count);

  ASSERT_EQ(1, m_calls.size());
  EXPECT_EQ("root", m_calls[0]);
}

TEST_F(node_key_observer_test, child_order)
{
  bim::axmol::input::node root;

  std::shared_ptr<key_observer_mockup> observer_1(
      std::make_shared<key_observer_mockup>("child_1", m_calls));
  root.push_back(observer_1);

  std::shared_ptr<key_observer_mockup> observer_2(
      std::make_shared<key_observer_mockup>("child_2", m_calls));
  root.push_back(observer_2);

  simulate_key_press(root);

  EXPECT_EQ(1, observer_1->m_pressed_call_count);
  EXPECT_EQ(1, observer_2->m_pressed_call_count);

  ASSERT_EQ(2, m_calls.size());
  EXPECT_EQ("child_1", m_calls[0]);
  EXPECT_EQ("child_2", m_calls[1]);
}

TEST_F(node_key_observer_test, depth_order)
{
  std::shared_ptr<key_observer_mockup> observer_1(
      std::make_shared<key_observer_mockup>("root", m_calls));
  bim::axmol::input::node root(observer_1);

  std::shared_ptr<key_observer_mockup> observer_2(
      std::make_shared<key_observer_mockup>("child", m_calls));
  bim::axmol::input::node_pointer child(
      new bim::axmol::input::node(observer_2));
  root.push_back(child);

  std::shared_ptr<key_observer_mockup> observer_3(
      std::make_shared<key_observer_mockup>("grandchild", m_calls));
  child->push_back(observer_3);

  simulate_key_press(root);

  EXPECT_EQ(1, observer_1->m_pressed_call_count);
  EXPECT_EQ(1, observer_2->m_pressed_call_count);
  EXPECT_EQ(1, observer_3->m_pressed_call_count);

  ASSERT_EQ(3, m_calls.size());
  EXPECT_EQ("grandchild", m_calls[0]);
  EXPECT_EQ("child", m_calls[1]);
  EXPECT_EQ("root", m_calls[2]);
}

TEST_F(node_key_observer_test, clear)
{
  bim::axmol::input::node root;

  std::shared_ptr<key_observer_mockup> observer_1(
      std::make_shared<key_observer_mockup>("child_1", m_calls));
  root.push_back(observer_1);

  std::shared_ptr<key_observer_mockup> observer_2(
      std::make_shared<key_observer_mockup>("child_2", m_calls));
  root.push_back(observer_2);

  root.clear();
  simulate_key_press(root);

  EXPECT_EQ(0, observer_1->m_pressed_call_count);
  EXPECT_EQ(0, observer_2->m_pressed_call_count);
  EXPECT_TRUE(m_calls.empty());
}

TEST_F(node_key_observer_test, uniqueness)
{
  bim::axmol::input::node root;

  std::shared_ptr<key_observer_mockup> observer_1(
      std::make_shared<key_observer_mockup>("child_1", m_calls));
  root.push_back(observer_1);

  std::shared_ptr<key_observer_mockup> observer_2(
      std::make_shared<key_observer_mockup>("child_2", m_calls));
  root.push_back(observer_2);

  EXPECT_TRUE(root.check_no_duplicates());

  root.attach(observer_2);
  EXPECT_FALSE(root.check_no_duplicates());
}
