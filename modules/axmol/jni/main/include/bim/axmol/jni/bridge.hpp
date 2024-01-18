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
