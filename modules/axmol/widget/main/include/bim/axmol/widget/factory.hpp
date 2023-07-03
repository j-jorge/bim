#pragma once

#include <bim/axmol/ref_ptr.hpp>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;

  template <typename T>
  class factory
  {
  public:
    /**
     * In the general case calling factory<T>::create(context, style) is
     * equivalent to T::create(context, style), but when T is a type from axmol
     * it does not have such function. For these types a specialization of
     * factory<T> allows to provide a single interface, whatever the origin of
     * the type.
     */
    [[nodiscard]] static bim::axmol::ref_ptr<T>
    create(const context&, const iscool::style::declaration&);
  };
};
