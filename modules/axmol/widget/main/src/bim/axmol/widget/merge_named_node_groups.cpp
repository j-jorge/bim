// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/merge_named_node_groups.hpp>

void bim::axmol::widget::merge_named_node_groups(named_node_group& d,
                                                 const named_node_group& s)
{
  d.insert(s.begin(), s.end());
}
