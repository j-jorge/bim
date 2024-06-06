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
