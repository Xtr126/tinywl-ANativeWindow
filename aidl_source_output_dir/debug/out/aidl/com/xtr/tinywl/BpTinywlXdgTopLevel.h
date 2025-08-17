/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/debug/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl5312348015633281126.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl/com/xtr/tinywl/ITinywlXdgTopLevel.aidl
 */
#pragma once

#include "aidl/com/xtr/tinywl/ITinywlXdgTopLevel.h"

#include <android/binder_ibinder.h>

namespace aidl {
namespace com {
namespace xtr {
namespace tinywl {
class BpTinywlXdgTopLevel : public ::ndk::BpCInterface<ITinywlXdgTopLevel> {
public:
  explicit BpTinywlXdgTopLevel(const ::ndk::SpAIBinder& binder);
  virtual ~BpTinywlXdgTopLevel();

  ::ndk::ScopedAStatus addXdgTopLevel(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
  ::ndk::ScopedAStatus removeXdgTopLevel(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
};
}  // namespace tinywl
}  // namespace xtr
}  // namespace com
}  // namespace aidl
