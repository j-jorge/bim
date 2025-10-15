// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_widget_create_function.hpp>
#include <bim/axmol/widget/named_node_group.hpp>

#include <bim/axmol/input/observer/scroll_view_glue_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <axmol/2d/Node.h>

#include <boost/unordered/unordered_flat_map.hpp>

namespace ax::extension
{
  class TableView;
}

namespace bim::axmol::style
{
  class bounds_properties;
}

namespace bim::axmol::widget
{
  /// A scrollable list of nodes.
  class list final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(list);

    list(const bim::axmol::widget::context& context,
         const iscool::style::declaration& style);
    ~list();

    void onEnter() override;
    void setContentSize(const ax::Size& size) override;

    bim::axmol::input::node_reference input_node() const;

    void push_back(ax::Node& node);
    void push_back(ax::Node& node, const iscool::style::declaration& size);

    void clear();

  private:
    class table_view_bridge;
    using custom_size_map =
        boost::unordered_flat_map<ax::Node*,
                                  const bim::axmol::style::bounds_properties*>;

  private:
    bool init() override;
    void update(float delta) override;

    void create_view();

    void set_dirty();
    void reload_data();

    void set_item_size(ax::Node& node);

  private:
    const bim::axmol::widget::context& m_context;

    bim::axmol::ref_ptr<ax::extension::TableView> m_table_view;
    const bim::axmol::ref_ptr<table_view_bridge> m_table_view_bridge;

    const bim::axmol::style::bounds_properties& m_default_item_size;
    custom_size_map m_item_size;

    std::unique_ptr<bim::axmol::input::scroll_view_glue_handle>
        m_scroll_view_inputs;
    bim::axmol::input::tree m_inputs;

    bool m_dirty;
  };
}
