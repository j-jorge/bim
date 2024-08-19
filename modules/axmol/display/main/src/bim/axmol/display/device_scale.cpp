// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/display/device_scale.hpp>

#include <bim/axmol/display/device_size.hpp>

#include <algorithm>

float bim::axmol::display::device_scale(float design_width,
                                        float design_height)
{
  const ax::Vec2 device = device_size();

  return std::min(device.x / design_width, device.y / design_height);
}
