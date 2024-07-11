#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>
#include <bim/axmol/widget/declare_widget_create_function.hpp>

#include <axmol/2d/Node.h>

namespace bim::axmol::widget
{
  class texture final : public ax::Node
  {
  public:
    bim_declare_widget_create_function(texture);

    texture(const bim::axmol::widget::context& context,
            const iscool::style::declaration& style);
    ~texture();

    void setContentSize(const ax::Size& size) override;

  private:
    class widgets;

  private:
    bool init() override;

  private:
    bim_declare_controls_struct(controls, m_controls, 1);
    const float m_device_scale;
  };
}
