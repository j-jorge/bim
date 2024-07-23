#pragma once

#include <bim/axmol/action/dynamic_factory.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/factory/dynamic_factory.impl.tpp>

#include <utility>

template <typename F>
void bim::axmol::action::dynamic_factory::register_action(
    const std::string& name, F&& f)
{
  m_factory.register_typename(name, std::forward<F>(f));
}
