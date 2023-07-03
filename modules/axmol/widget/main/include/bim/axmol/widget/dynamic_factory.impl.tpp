#pragma once

#include <bim/axmol/widget/factory.hpp>

template <typename T>
void bim::axmol::widget::dynamic_factory::register_widget(
    const std::string& type)
{
  m_factory.register_typename(
      type,
      [](const context& c, const iscool::style::declaration& style)
          -> bim::axmol::ref_ptr<ax::Node>
      {
        // TODO: fix ref_ptr implementation in axmol to allow implicit cast.
        return ax::static_pointer_cast<ax::Node>(factory<T>::create(c, style));
      });
}
