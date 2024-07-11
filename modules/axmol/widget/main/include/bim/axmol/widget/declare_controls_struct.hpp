#pragma once

#include <bim/axmol/widget/named_node_group.hpp>

#include <iscool/memory/pimpl.hpp>

// There are many widgets in UI and each of them tend to have many
// components. If we implement them naively, by listing them all in the header,
// we end up having a strong coupling between the component list and the
// including headers. Since the component list tend to evolve during
// development, this will trigger many recompilations. To avoid this we will
// implement the controls struct via the PIMPL idiom.
//
// Now, In order to reduce the amount of dynamic allocations at start up, as
// well as the number of indirections, we will use the PIMPL version that
// embeds its data, but then we must know how much space we need. The size of
// this space is defined by the implementation from
// implement_controls_struct.hpp:
//   - one bim::axmol::ref_ptr<T> per component.
//   - one bim::axmol::widget::named_node_group overall.
constexpr size_t ref_ptr_size = sizeof(void*);

#define bim_declare_controls_struct(type, member, count)                      \
  struct type;                                                                \
  const iscool::memory::pimpl<                                                \
      type, count * sizeof(bim::axmol::ref_ptr<ax::Node>)                     \
                + sizeof(bim::axmol::widget::named_node_group)>               \
      member
