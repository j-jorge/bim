// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/jni/bridge.hpp>

#include <iscool/jni/setup.hpp>

#include <axmol/platform/android/jni/JniHelper.h>

bim::axmol::jni::bridge::bridge()
{
  iscool::jni::initialize(&ax::JniHelper::getEnv);
}

bim::axmol::jni::bridge::~bridge()
{
  iscool::jni::finalize();
}
