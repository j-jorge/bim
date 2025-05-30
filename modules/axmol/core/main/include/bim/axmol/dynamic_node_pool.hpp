// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/ref_ptr.hpp>

#include <iscool/memory/dynamic_pool.hpp>

namespace bim::axmol
{
  namespace detail
  {
    template <typename T>
    struct dynamic_node_pool_traits
    {
      static bool clear(const ref_ptr<T>& p)
      {
        if (p->getParent() != nullptr)
          p->removeFromParent();

        return true;
      }
    };
  }

  template <typename T>
  using dynamic_node_pool =
      iscool::memory::dynamic_pool<ref_ptr<T>,
                                   detail::dynamic_node_pool_traits<T>>;
}
