/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/debug/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl5312348015633281126.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl/com/xtr/tinywl/ITinywlXdgTopLevel.aidl
 */
#pragma once

#include "aidl/com/xtr/tinywl/ITinywlXdgTopLevel.h"

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
class BnTinywlXdgTopLevel : public ::ndk::BnCInterface<ITinywlXdgTopLevel> {
public:
  BnTinywlXdgTopLevel();
  virtual ~BnTinywlXdgTopLevel();
protected:
  ::ndk::SpAIBinder createBinder() override;
private:
};
class ITinywlXdgTopLevelDelegator : public BnTinywlXdgTopLevel {
public:
  explicit ITinywlXdgTopLevelDelegator(const std::shared_ptr<ITinywlXdgTopLevel> &impl) : _impl(impl) {
  }

  ::ndk::ScopedAStatus addXdgTopLevel(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override {
    return _impl->addXdgTopLevel(in_xdgToplevel);
  }
  ::ndk::ScopedAStatus removeXdgTopLevel(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override {
    return _impl->removeXdgTopLevel(in_xdgToplevel);
  }
protected:
private:
  std::shared_ptr<ITinywlXdgTopLevel> _impl;
};

}  // namespace tinywl
}  // namespace xtr
}  // namespace com
}  // namespace aidl
