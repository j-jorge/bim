// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <bim/axmol/input/observer/tap_observer_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <bim/axmol/ref_ptr.hpp>

#include <iscool/signals/declare_signal.hpp>

#include <axmol/2d/Node.h>

#include <string_view>

namespace ax::ui
{
  class EditBoxDelegate;
}

namespace bim::axmol::widget
{
  class edit_box final : public ax::Node
  {
    class Delegate;
    friend class Delegate;

    DECLARE_VOID_SIGNAL(changed, m_changed)
    DECLARE_VOID_SIGNAL(cancel, m_cancel)

  public:
    bim_declare_widget_create_function(edit_box);

    edit_box(const bim::axmol::widget::context& context,
             const iscool::style::declaration& style);
    ~edit_box();

    bim::axmol::input::node_reference input_node() const;

    void enable(bool enabled);

    void set_text(const char* text);
    std::string_view get_text() const;

    void max_length(int n);

    void onEnter() override;
    void setContentSize(const ax::Size& size) override;

  private:
    class widgets;

  private:
    bool init() override;

    void update_bounds();

  private:
    const bim::axmol::widget::context& m_context;
    bim_declare_controls_struct(controls, m_controls, 1);
    const bim::axmol::input::tap_observer_handle m_tap_observer;
    bim::axmol::input::tree m_inputs;

    const iscool::style::declaration& m_style_bounds;

    std::unique_ptr<ax::ui::EditBoxDelegate> m_delegate;

    bool m_bounds_dirty;
  };
}
