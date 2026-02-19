// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace bim::game
{
  class animation_catalog;
  class context;

  void register_crate_animations(context& context,
                                 animation_catalog& animations);
}
