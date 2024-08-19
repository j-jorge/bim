// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace bim::axmol::widget
{
  class dynamic_factory;

  /**
   * Register all widgets from this library into the dynamic factory, such that
   * they can be instantiated from a style file.
   */
  void register_widgets(dynamic_factory& factory);
}
