// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/application.hpp>
#include <bim/axmol/app/bridge.hpp>

#include <bim/axmol/jni/bridge.hpp>

#include <iscool/log/enable_console_log.hpp>

#include <jni.h>

namespace
{
  struct application
  {
    application()
      : bridge(new bim::axmol::app::bridge())
    {}

    ~application()
    {
      // The bridge must be destroyed before the app becauses it accesses the
      // global scheduler, which is destroyed with the app.
      bridge.reset();
    }

    std::unique_ptr<bim::axmol::app::bridge> bridge;
    bim::axmol::jni::bridge jni;
    bim::axmol::app::application instance;
  };
}

static std::unique_ptr<application> g_application;

void axmol_android_app_init(JNIEnv* env)
{
  iscool::log::enable_console_log();
  g_application.reset(new application());
}
