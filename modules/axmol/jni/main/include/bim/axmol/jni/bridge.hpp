// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace bim::axmol::jni
{
  class bridge
  {
  public:
    bridge();
    ~bridge();

    bridge(bridge&&) = delete;
    bridge& operator=(bridge&&) = delete;
  };
}
