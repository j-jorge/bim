// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/dynamic_factory.hpp>
#include <bim/axmol/widget/factory.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/factory/dynamic_factory.impl.tpp>

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
