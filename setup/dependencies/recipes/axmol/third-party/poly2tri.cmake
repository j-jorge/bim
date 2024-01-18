# The link fails with missing symbols when poly2tri is built as a
# separate static library, so we include it in libaxmol.

set(poly2tri_include_dirs ${third_party_root}/poly2tri)
set(poly2tri_sources
  ${third_party_root}/poly2tri/common/shapes.cc
  ${third_party_root}/poly2tri/sweep/advancing_front.cc
  ${third_party_root}/poly2tri/sweep/cdt.cc
  ${third_party_root}/poly2tri/sweep/sweep.cc
  ${third_party_root}/poly2tri/sweep/sweep_context.cc
)
