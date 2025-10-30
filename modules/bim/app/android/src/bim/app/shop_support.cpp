// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/shop_support.hpp>

#include <iscool/jni/get_static_method.hpp>
#include <iscool/jni/static_method_jboolean.hpp>

bool bim::app::is_shop_supported()
{
  static bool result = []() -> bool
  {
    iscool::jni::static_method<jboolean> method(
        iscool::jni::get_static_method<jboolean>("bim/app/ShopService",
                                                 "supported", "()Z"));

    return method();
  }();

  return result;
}
