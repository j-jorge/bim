// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace bim::axmol::http
{
  class bridge
  {
  public:
    bridge();
    ~bridge();

    bridge(const bridge&) = delete;
    bridge& operator=(const bridge&) = delete;
  };
}
