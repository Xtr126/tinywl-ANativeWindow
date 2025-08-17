/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/debug/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl14536866063595600578.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl/com/xtr/tinywl/ITinywlCallback.aidl
 */
#pragma once

#include "aidl/com/xtr/tinywl/ITinywlCallback.h"

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
class BnTinywlCallback : public ::ndk::BnCInterface<ITinywlCallback> {
public:
  BnTinywlCallback();
  virtual ~BnTinywlCallback();
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class ITinywlCallbackDelegator : public BnTinywlCallback {
public:
  explicit ITinywlCallbackDelegator(const std::shared_ptr<ITinywlCallback> &impl) : _impl(impl) {
  }

  ::ndk::ScopedAStatus startTinywl(const std::shared_ptr<::aidl::com::xtr::tinywl::ITinywlXdgTopLevel>& in_xdgTopLevelCallback) override {
    return _impl->startTinywl(in_xdgTopLevelCallback);
  }
  ::ndk::ScopedAStatus onSurfaceCreated(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override {
    return _impl->onSurfaceCreated(in_xdgToplevel);
  }
  ::ndk::ScopedAStatus onSurfaceChanged(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override {
    return _impl->onSurfaceChanged(in_xdgToplevel);
  }
  ::ndk::ScopedAStatus onSurfaceDestroyed(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override {
    return _impl->onSurfaceDestroyed(in_xdgToplevel);
  }
protected:
private:
  std::shared_ptr<ITinywlCallback> _impl;
};

}  // namespace tinywl
}  // namespace xtr
}  // namespace com
}  // namespace aidl
