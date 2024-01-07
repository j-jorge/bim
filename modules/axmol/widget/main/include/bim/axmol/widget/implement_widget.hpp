#pragma once

#include <bim/axmol/widget/factory.impl.tpp>

namespace iscool::style
{
  class declaration;
}

/**
 * Implement the wigdet::create function with the signature required by the
 * widget factory, and implement the widget factory for the given widget.
 */
#define bim_implement_widget(type)                                            \
  [[nodiscard]] bim::axmol::ref_ptr<type> type::create(                       \
      const bim::axmol::widget::context& context,                             \
      const iscool::style::declaration& style)                                \
  {                                                                           \
    type* const result = new type(context, style);                            \
    result->autorelease();                                                    \
    result->init();                                                           \
                                                                              \
    return result;                                                            \
  }                                                                           \
                                                                              \
  template class bim::axmol::widget::factory<type>
