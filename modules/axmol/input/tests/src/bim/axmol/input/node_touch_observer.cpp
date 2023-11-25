#include <bim/axmol/input/node.hpp>
#include <bim/axmol/input/node_reference.hpp>
#include <bim/axmol/input/touch_event_view.hpp>
#include <bim/axmol/input/touch_observer.hpp>

#include <axmol/base/Touch.h>

#include <gtest/gtest.h>

class call_tracker
{
public:
  void clear();

public:
  std::vector<std::string> pressed;
  std::vector<std::string> moved;
  std::vector<std::string> released;
  std::vector<std::string> cancelled;
};

namespace
{
  class touch_observer_mockup : public bim::axmol::input::touch_observer
  {
  public:
    touch_observer_mockup(std::string name, call_tracker& calls);

  private:
    bool do_is_relevant_to_pressed(
        const bim::axmol::input::touch_event_view& touches) const override;

    void
    do_pressed(const bim::axmol::input::touch_event_view& touches) override;
    void do_moved(const bim::axmol::input::touch_event_view& touches) override;
    void
    do_released(const bim::axmol::input::touch_event_view& touches) override;
    void
    do_cancelled(const bim::axmol::input::touch_event_view& touches) override;

  public:
    std::size_t m_pressed_call_count;
    bool m_is_relevant_to_pressed_result;

  private:
    const std::string m_name;
    call_tracker& m_calls;
  };
}

void call_tracker::clear()
{
  pressed.clear();
  moved.clear();
  released.clear();
  cancelled.clear();
}

touch_observer_mockup::touch_observer_mockup(std::string name,
                                             call_tracker& calls)
  : m_pressed_call_count(0)
  , m_is_relevant_to_pressed_result(true)
  , m_name(std::move(name))
  , m_calls(calls)
{}

bool touch_observer_mockup::do_is_relevant_to_pressed(
    const bim::axmol::input::touch_event_view& touches) const
{
  return m_is_relevant_to_pressed_result;
}

void touch_observer_mockup::do_pressed(
    const bim::axmol::input::touch_event_view& touches)
{
  m_calls.pressed.push_back(m_name);
  ++m_pressed_call_count;
}

void touch_observer_mockup::do_moved(
    const bim::axmol::input::touch_event_view& touches)
{
  m_calls.moved.push_back(m_name);
}

void touch_observer_mockup::do_released(
    const bim::axmol::input::touch_event_view& touches)
{
  m_calls.released.push_back(m_name);
}

void touch_observer_mockup::do_cancelled(
    const bim::axmol::input::touch_event_view& touches)
{
  m_calls.cancelled.push_back(m_name);
}

class node_touch_observer_test : public ::testing::Test
{
protected:
  call_tracker m_calls;
};

static void invoke_pressed(bim::axmol::input::node& node)
{
  ax::Touch touch;
  bim::axmol::input::touch_event event(&touch);
  bim::axmol::input::touch_event_view touches(std::span(&event, 1));
  node.touch_pressed(touches);
}

static void invoke_moved(bim::axmol::input::node& node)
{
  ax::Touch touch;
  bim::axmol::input::touch_event event(&touch);
  bim::axmol::input::touch_event_view touches(std::span(&event, 1));
  node.touch_moved(touches);
}

TEST_F(node_touch_observer_test, invoke_self)
{
  std::shared_ptr<touch_observer_mockup> observer(
      std::make_shared<touch_observer_mockup>("root", m_calls));
  bim::axmol::input::node node(observer);

  invoke_pressed(node);

  EXPECT_EQ(1, observer->m_pressed_call_count);

  ASSERT_EQ(1, m_calls.pressed.size());
  EXPECT_TRUE(m_calls.moved.empty());
  EXPECT_TRUE(m_calls.released.empty());
  EXPECT_TRUE(m_calls.cancelled.empty());

  EXPECT_EQ("root", m_calls.pressed[0]);
}

TEST_F(node_touch_observer_test, children_order)
{
  bim::axmol::input::node root;

  std::shared_ptr<touch_observer_mockup> observer_1(
      std::make_shared<touch_observer_mockup>("child_1", m_calls));
  root.push_back(observer_1);

  std::shared_ptr<touch_observer_mockup> observer_2(
      std::make_shared<touch_observer_mockup>("child_2", m_calls));
  root.push_back(observer_2);

  invoke_pressed(root);

  EXPECT_EQ(1, observer_1->m_pressed_call_count);
  EXPECT_EQ(1, observer_2->m_pressed_call_count);

  ASSERT_EQ(2, m_calls.pressed.size());
  EXPECT_TRUE(m_calls.moved.empty());
  EXPECT_TRUE(m_calls.released.empty());
  EXPECT_TRUE(m_calls.cancelled.empty());

  EXPECT_EQ("child_1", m_calls.pressed[0]);
  EXPECT_EQ("child_2", m_calls.pressed[1]);
}

TEST_F(node_touch_observer_test, depth_order)
{
  std::shared_ptr<touch_observer_mockup> observer_1(
      std::make_shared<touch_observer_mockup>("root", m_calls));
  bim::axmol::input::node root(observer_1);

  std::shared_ptr<touch_observer_mockup> observer_2(
      std::make_shared<touch_observer_mockup>("child", m_calls));
  bim::axmol::input::node_pointer child(
      new bim::axmol::input::node(observer_2));
  root.push_back(child);

  std::shared_ptr<touch_observer_mockup> observer_3(
      std::make_shared<touch_observer_mockup>("grandchild", m_calls));
  child->push_back(observer_3);

  invoke_pressed(root);

  EXPECT_EQ(1, observer_1->m_pressed_call_count);
  EXPECT_EQ(1, observer_2->m_pressed_call_count);
  EXPECT_EQ(1, observer_3->m_pressed_call_count);

  ASSERT_EQ(3, m_calls.pressed.size());
  EXPECT_TRUE(m_calls.moved.empty());
  EXPECT_TRUE(m_calls.released.empty());
  EXPECT_TRUE(m_calls.cancelled.empty());

  EXPECT_EQ("grandchild", m_calls.pressed[0]);
  EXPECT_EQ("child", m_calls.pressed[1]);
  EXPECT_EQ("root", m_calls.pressed[2]);
}

TEST_F(node_touch_observer_test, clear)
{
  bim::axmol::input::node root;

  std::shared_ptr<touch_observer_mockup> observer_1(
      std::make_shared<touch_observer_mockup>("child_1", m_calls));
  root.push_back(observer_1);

  std::shared_ptr<touch_observer_mockup> observer_2(
      std::make_shared<touch_observer_mockup>("child_2", m_calls));
  root.push_back(observer_2);

  root.clear();
  invoke_pressed(root);

  EXPECT_EQ(0, observer_1->m_pressed_call_count);
  EXPECT_EQ(0, observer_2->m_pressed_call_count);

  EXPECT_TRUE(m_calls.pressed.empty());
  EXPECT_TRUE(m_calls.moved.empty());
  EXPECT_TRUE(m_calls.released.empty());
  EXPECT_TRUE(m_calls.cancelled.empty());
}

TEST_F(node_touch_observer_test, uniqueness)
{
  bim::axmol::input::node root;

  std::shared_ptr<touch_observer_mockup> observer_1(
      std::make_shared<touch_observer_mockup>("child_1", m_calls));
  root.push_back(observer_1);
  std::shared_ptr<touch_observer_mockup> observer_2(
      std::make_shared<touch_observer_mockup>("child_2", m_calls));
  root.push_back(observer_2);

  EXPECT_TRUE(root.check_no_duplicates());

  root.attach(observer_2);
  EXPECT_FALSE(root.check_no_duplicates());
}

TEST_F(node_touch_observer_test, is_relevant_to_pressed)
{
  /*
    root
    |- 1
    |- 2
    |  |- 2a
    |  |- 2b
    |- 3
  */
  std::shared_ptr<touch_observer_mockup> observer_root(
      std::make_shared<touch_observer_mockup>("root", m_calls));
  bim::axmol::input::node root(observer_root);

  std::shared_ptr<touch_observer_mockup> observer_1(
      std::make_shared<touch_observer_mockup>("child 1", m_calls));
  root.push_back(observer_1);

  std::shared_ptr<touch_observer_mockup> observer_2(
      std::make_shared<touch_observer_mockup>("child 2", m_calls));
  bim::axmol::input::node_pointer child_2(
      new bim::axmol::input::node(observer_2));
  root.push_back(child_2);

  std::shared_ptr<touch_observer_mockup> observer_2a(
      std::make_shared<touch_observer_mockup>("grandchild 2a", m_calls));
  child_2->push_back(observer_2a);

  std::shared_ptr<touch_observer_mockup> observer_2b(
      std::make_shared<touch_observer_mockup>("grandchild 2b", m_calls));
  child_2->push_back(observer_2b);

  std::shared_ptr<touch_observer_mockup> observer_3(
      std::make_shared<touch_observer_mockup>("child 3", m_calls));
  root.push_back(observer_3);

  observer_2->m_is_relevant_to_pressed_result = false;

  invoke_pressed(root);

  ASSERT_EQ(3, m_calls.pressed.size());
  EXPECT_TRUE(m_calls.moved.empty());
  EXPECT_TRUE(m_calls.released.empty());
  EXPECT_TRUE(m_calls.cancelled.empty());

  EXPECT_EQ("child 1", m_calls.pressed[0]);
  EXPECT_EQ("child 3", m_calls.pressed[1]);
  EXPECT_EQ("root", m_calls.pressed[2]);

  m_calls.clear();

  invoke_moved(root);

  EXPECT_TRUE(m_calls.pressed.empty());
  ASSERT_EQ(3, m_calls.moved.size());
  EXPECT_TRUE(m_calls.released.empty());
  EXPECT_TRUE(m_calls.cancelled.empty());

  EXPECT_EQ("child 1", m_calls.moved[0]);
  EXPECT_EQ("child 3", m_calls.moved[1]);
  EXPECT_EQ("root", m_calls.moved[2]);
}
