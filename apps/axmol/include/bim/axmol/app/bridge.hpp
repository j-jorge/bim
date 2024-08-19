// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/files/bridge.hpp>
#include <bim/axmol/http/bridge.hpp>
#include <bim/axmol/schedule/bridge.hpp>

namespace bim::axmol::app
{
  class bridge
  {
  public:
    bridge();
    bridge(const bridge&) = delete;
    bridge(bridge&&) = delete;

    bridge& operator=(const bridge&) = delete;
    bridge& operator=(bridge&&) = delete;

  private:
    bim::axmol::files::bridge m_files;
    bim::axmol::http::bridge m_http;
    bim::axmol::schedule::bridge m_schedule;
  };
}
