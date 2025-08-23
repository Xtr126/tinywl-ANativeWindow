/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out/ndk -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out/ndk -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/debug/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl17421876105909342718.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl/com/xtr/tinywl/TinywlXdgTopLevelCallback.aidl
 */
#pragma once

#include "aidl/com/xtr/tinywl/TinywlXdgTopLevelCallback.h"

#include <android/binder_ibinder.h>
#include <cassert>

#ifndef __BIONIC__
#ifndef __assert2
#define __assert2(a,b,c,d) ((void)0)
#endif
#endif

namespace aidl {
namespace com {
namespace xtr {
namespace tinywl {
class BnTinywlXdgTopLevelCallback : public ::ndk::BnCInterface<ITinywlXdgTopLevelCallback> {
public:
  BnTinywlXdgTopLevelCallback();
  virtual ~BnTinywlXdgTopLevelCallback();
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class ITinywlXdgTopLevelCallbackDelegator : public BnTinywlXdgTopLevelCallback {
public:
  explicit ITinywlXdgTopLevelCallbackDelegator(const std::shared_ptr<ITinywlXdgTopLevelCallback> &impl) : _impl(impl) {
  }

protected:
private:
  std::shared_ptr<ITinywlXdgTopLevelCallback> _impl;
};

}  // namespace tinywl
}  // namespace xtr
}  // namespace com
}  // namespace aidl
