// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/action/factory/delay.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/style/declaration.hpp>

#include <axmol/2d/ActionInterval.h>

bim::axmol::ref_ptr<ax::ActionInterval>
bim::axmol::action::delay_from_style(const iscool::style::declaration& style)
{
  return ax::DelayTime::create(*style.get_number("duration"));
}
