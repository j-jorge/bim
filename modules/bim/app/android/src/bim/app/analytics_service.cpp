// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/analytics_service.hpp>

#include <iscool/jni/get_static_method.hpp>
#include <iscool/jni/hash_map.hpp>
#include <iscool/jni/new_java_string.hpp>
#include <iscool/jni/static_method_void.hpp>

bim::app::analytics_service::analytics_service() = default;
bim::app::analytics_service::~analytics_service() = default;

void bim::app::analytics_service::event(std::string_view name)
{
  const iscool::jni::static_method<void> method(
      iscool::jni::get_static_method<void>("bim/app/AnalyticsService", "event",
                                           "(Ljava/lang/String;)V"));
  method(iscool::jni::new_java_string(std::string(name)));
}

void bim::app::analytics_service::event(std::string_view name,
                                        const property_list& properties)
{
  const iscool::jni::static_method<void> method(
      iscool::jni::get_static_method<void>(
          "bim/app/AnalyticsService", "event",
          "(Ljava/lang/String;Ljava/util/Map;)V"));

  iscool::jni::hash_map<jstring, jstring> java_properties;

  for (const std::pair<std::string_view, std::string_view>& e : properties)
    java_properties.put(iscool::jni::new_java_string(std::string(e.first)),
                        iscool::jni::new_java_string(std::string(e.second)));

  method(iscool::jni::new_java_string(std::string(name)), java_properties);
}

void bim::app::analytics_service::screen(std::string_view name)
{
  const iscool::jni::static_method<void> method(
      iscool::jni::get_static_method<void>("bim/app/AnalyticsService",
                                           "screen", "(Ljava/lang/String;)V"));
  method(iscool::jni::new_java_string(std::string(name)));
}

void bim::app::analytics_service::screen(std::string_view name,
                                         const property_list& properties)
{
  const iscool::jni::static_method<void> method(
      iscool::jni::get_static_method<void>(
          "bim/app/AnalyticsService", "screen",
          "(Ljava/lang/String;Ljava/util/Map;)V"));

  iscool::jni::hash_map<jstring, jstring> java_properties;

  for (const std::pair<std::string_view, std::string_view>& e : properties)
    java_properties.put(iscool::jni::new_java_string(std::string(e.first)),
                        iscool::jni::new_java_string(std::string(e.second)));

  method(iscool::jni::new_java_string(std::string(name)), java_properties);
}
