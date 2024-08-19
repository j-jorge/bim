// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;
}

#define bim_declare_widget_create_function(type)                              \
  [[nodiscard]] static bim::axmol::ref_ptr<type> create(                      \
      const bim::axmol::widget::context&, const iscool::style::declaration&)
