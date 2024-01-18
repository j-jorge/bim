#include <bim/axmol/app/application.hpp>
#include <bim/axmol/app/bridge.hpp>

#include <bim/axmol/jni/bridge.hpp>

#include <jni.h>

namespace
{
  struct application
  {
    bim::axmol::app::bridge bridge;
    bim::axmol::jni::bridge jni;
    bim::axmol::app::application instance;
  };
}

static std::unique_ptr<application> g_application;

void cocos_android_app_init(JNIEnv* env)
{
  g_application.reset(new application());
}
