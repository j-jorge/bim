#pragma once

#include <bim/axmol/widget/factory.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

template <typename T>
void bim::axmol::widget::dynamic_factory::register_widget(
    const std::string& type)
{
  m_factory.register_typename(
      type,
      [](const context& c, const iscool::style::declaration& style)
          -> bim::axmol::ref_ptr<ax::Node>
      {
        return factory<T>::create(c, style);
      });
}
