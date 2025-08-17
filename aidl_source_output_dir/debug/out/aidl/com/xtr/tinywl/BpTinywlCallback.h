/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/debug/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl14536866063595600578.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl/com/xtr/tinywl/ITinywlCallback.aidl
 */
#pragma once

#include "aidl/com/xtr/tinywl/ITinywlCallback.h"

#include <android/binder_ibinder.h>

namespace aidl {
namespace com {
namespace xtr {
namespace tinywl {
class BpTinywlCallback : public ::ndk::BpCInterface<ITinywlCallback> {
public:
  explicit BpTinywlCallback(const ::ndk::SpAIBinder& binder);
  virtual ~BpTinywlCallback();

  ::ndk::ScopedAStatus startTinywl(const std::shared_ptr<::aidl::com::xtr::tinywl::ITinywlXdgTopLevel>& in_xdgTopLevelCallback) override;
  ::ndk::ScopedAStatus onSurfaceCreated(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
  ::ndk::ScopedAStatus onSurfaceChanged(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
  ::ndk::ScopedAStatus onSurfaceDestroyed(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
};
}  // namespace tinywl
}  // namespace xtr
}  // namespace com
}  // namespace aidl
