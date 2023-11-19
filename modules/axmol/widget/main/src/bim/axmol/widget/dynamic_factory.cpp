#include <bim/axmol/widget/dynamic_factory.hpp>

#include <iscool/factory/dynamic_factory.impl.tpp>

bim::axmol::ref_ptr<ax::Node> bim::axmol::widget::dynamic_factory::create(
    std::string_view type, const context& c,
    const iscool::style::declaration& style) const
{
  return m_factory.create_by_typename(type, c, style);
}
