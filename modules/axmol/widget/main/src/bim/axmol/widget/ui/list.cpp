// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/ui/list.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>
#include <bim/axmol/widget/implement_widget.hpp>

#include <bim/axmol/input/observer/scroll_view_glue.hpp>
#include <bim/axmol/input/touch_observer_handle.impl.hpp>

#include <bim/axmol/style/apply_size.hpp>

#include <axmol/extensions/GUI/ScrollView/TableView.h>

#include <iscool/style/declaration.hpp>

#include <vector>

class bim::axmol::widget::list::table_view_bridge final
  : public ax::Object
  , public ax::extension::TableViewDataSource
  , public ax::extension::TableViewDelegate
{
public:
  void push_back(ax::Node& item)
  {
    item.setAnchorPoint(ax::Vec2(0, 0));
    m_items.push_back(&item);
  }

  void remove_all_from_parents()
  {
    for (const bim::axmol::ref_ptr<ax::Node>& item : m_items)
      item->removeFromParent();
  }

  void clear()
  {
    m_items.clear();
  }

  std::size_t item_count() const
  {
    return m_items.size();
  }

  ax::Node& at(std::size_t i) const
  {
    return *m_items[i];
  }

private:
  // Functions from TableViewDataSource.
  ax::Size tableCellSizeForIndex(ax::extension::TableView* table,
                                 ssize_t index) override
  {
    return m_items[index]->getContentSize();
  }

  ax::extension::TableViewCell*
  tableCellAtIndex(ax::extension::TableView* table, ssize_t index) override
  {
    ax::extension::TableViewCell* cell(table->cellAtIndex(index));

    // The cell is in the table, use it.
    if (cell != nullptr)
      return cell;

    // The cell is not in the table, try to recycle an old one, or create a new
    // one if needed.
    cell = table->dequeueCell();

    if (cell == nullptr)
      {
        cell = new ax::extension::TableViewCell();
        cell->autorelease();
      }

    assert(cell->getChildrenCount() == 0);

    // Put the client node into the cell.
    ax::Node* const node = m_items[index].get();
    assert(node != nullptr);

    cell->addChild(node);

    return cell;
  }

  ssize_t numberOfCellsInTableView(ax::extension::TableView* table) override
  {
    return m_items.size();
  }

  // Functions from TableViewDelegate.
  void tableCellTouched(ax::extension::TableView* table,
                        ax::extension::TableViewCell* cell) override
  {
    // Nothing to do, it is handled in our input tree.
  }

  void tableCellWillRecycle(ax::extension::TableView* table,
                            ax::extension::TableViewCell* cell) override
  {
    cell->removeAllChildren();
  }

private:
  std::vector<bim::axmol::ref_ptr<ax::Node>> m_items;
};

bim_implement_widget(bim::axmol::widget::list);

bim::axmol::widget::list::list(const bim::axmol::widget::context& context,
                               const iscool::style::declaration& style)
  : m_context(context)
  , m_table_view_bridge(new table_view_bridge())
  , m_item_size(*style.get_declaration("item-size"))
{
  m_table_view_bridge->autorelease();
}

bim::axmol::widget::list::~list() = default;

void bim::axmol::widget::list::onEnter()
{
  ax::Node::onEnter();

  reload_data();
}

void bim::axmol::widget::list::setContentSize(const ax::Size& size)
{
  if (size.equals(getContentSize()))
    return;

  ax::Node::setContentSize(size);

  // setContentSize is called from init(), but we don't want to create the
  // table here, we only want to update it if it exist.
  if (!m_table_view)
    return;

  // There is no such thing as resizing a table view. The size is passed to the
  // constructor so we have to remove everything and create a new view.
  m_table_view_bridge->remove_all_from_parents();

  m_table_view->removeFromParent();
  m_table_view = nullptr;

  m_inputs.clear();
  m_scroll_view_inputs = {};

  for (std::size_t i = 0, n = m_table_view_bridge->item_count(); i != n; ++i)
    set_item_size(m_table_view_bridge->at(i));

  create_view();
}

bim::axmol::input::node_reference bim::axmol::widget::list::input_node() const
{
  return m_inputs.root();
}

void bim::axmol::widget::list::push_back(ax::Node& node)
{
  set_item_size(node);
  m_table_view_bridge->push_back(node);
  set_dirty();
}

void bim::axmol::widget::list::clear()
{
  m_table_view_bridge->clear();
  reload_data();
}

bool bim::axmol::widget::list::init()
{
  if (!ax::Node::init())
    return false;

  create_view();

  return true;
}

void bim::axmol::widget::list::update(float delta)
{
  assert(m_dirty);

  ax::Node::update(delta);

  m_dirty = false;
  unscheduleUpdate();
  reload_data();
}

void bim::axmol::widget::list::create_view()
{
  assert(!m_table_view);
  assert(m_table_view_bridge);

  const ax::Size size = getContentSize();

  m_table_view =
      ax::extension::TableView::create(m_table_view_bridge.get(), size);

  m_table_view->setDelegate(m_table_view_bridge.get());
  m_table_view->reloadData();

  m_table_view->setBounceable(true);
  m_table_view->setDirection(ax::extension::TableView::Direction::VERTICAL);
  m_table_view->setPosition(ax::Vec2(0, 0));
  m_table_view->setVerticalFillOrder(
      ax::extension::TableView::VerticalFillOrder::TOP_DOWN);
  m_table_view->setViewSize(size);

  addChild(m_table_view.get());

  m_scroll_view_inputs =
      bim::axmol::input::scroll_view_glue_handle(*m_table_view);

  m_inputs.attach_to_root(m_scroll_view_inputs);
}

void bim::axmol::widget::list::set_dirty()
{
  if (m_dirty)
    return;

  m_dirty = true;

  if (isRunning())
    scheduleUpdate();
}

void bim::axmol::widget::list::reload_data()
{
  if (!isRunning())
    {
      m_dirty = true;
      return;
    }

  if (!m_dirty)
    return;

  m_dirty = false;

  // Reloading the data resets the scroll position. We store the offset to
  // restore it after the reload.

  const float view_height = m_table_view->getViewSize().height;
  const ax::Vec2 view_offset = m_table_view->getContentOffset();
  const float old_top =
      m_table_view->getContentSize().height - view_height + view_offset.y;

  m_table_view->reloadData();

  const float new_top =
      std::max(0.f, m_table_view->getContentSize().height - old_top);
  m_table_view->setContentOffset(
      ax::Vec2(view_offset.x, -(new_top - view_height)));
}

void bim::axmol::widget::list::set_item_size(ax::Node& node)
{
  bim::axmol::style::apply_size(node, *this,
                                m_context.style_cache.get_bounds(m_item_size));
}
