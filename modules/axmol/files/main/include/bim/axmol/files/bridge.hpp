// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <memory>

namespace bim::axmol::files
{
  class bridge
  {
  public:
    bridge();
    ~bridge();

    bridge(bridge&&) = delete;
    bridge& operator=(bridge&&) = delete;

  private:
    class delegates;

  private:
    std::unique_ptr<delegates> m_delegates;
  };
}
