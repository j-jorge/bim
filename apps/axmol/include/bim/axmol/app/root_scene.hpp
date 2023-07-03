#pragma once

#include <iscool/signals/declare_signal.hpp>

#include <axmol/2d/Scene.h>

namespace bim::axmol::app
{
  /**
   * The main scene passed to axmol::Director. This is just a proxy to catch
   * the clean-up on exit and signal it to the application.
   */
  class root_scene : public ax::Scene
  {
    DECLARE_VOID_SIGNAL(clean_up, m_clean_up)

  public:
    static root_scene* create();

  private:
    void cleanup() override;
  };
}
