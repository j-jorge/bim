// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/jni/hash_map.hpp>
#include <iscool/jni/scoped_native_callback.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace bim::app
{
  class shop_implementation
  {
    DECLARE_SIGNAL(void(const std::unordered_map<std::string, std::string>&),
                   products_ready, m_products_ready)
    DECLARE_VOID_SIGNAL(products_error, m_products_error)
    DECLARE_SIGNAL(void(std::string_view, std::size_t, std::string_view),
                   purchase_completed, m_purchase_completed)
    DECLARE_VOID_SIGNAL(purchase_error, m_purchase_error)

  public:
    shop_implementation();
    ~shop_implementation();

    void fetch_products(std::span<const std::string_view> ids);
    void refresh_purchases();
    void purchase(std::string_view id);

  private:
    void dispatch_products_ready(
        const iscool::jni::hash_map<jstring, jstring>& products) const;

  private:
    iscool::jni::scoped_native_callback m_products_ready_from_java;
    iscool::jni::scoped_native_callback m_products_error_from_java;
    iscool::jni::scoped_native_callback m_purchase_completed_from_java;
    iscool::jni::scoped_native_callback m_purchase_error_from_java;
  };
}
