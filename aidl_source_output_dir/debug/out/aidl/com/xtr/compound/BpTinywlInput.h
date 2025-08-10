/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl8668083705324803605.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl/com/xtr/compound/ITinywlInput.aidl
 */
#pragma once

#include "aidl/com/xtr/compound/ITinywlInput.h"

#include <android/binder_ibinder.h>

namespace aidl {
namespace com {
namespace xtr {
namespace compound {
class BpTinywlInput : public ::ndk::BpCInterface<ITinywlInput> {
public:
  explicit BpTinywlInput(const ::ndk::SpAIBinder& binder);
  virtual ~BpTinywlInput();

  ::ndk::ScopedAStatus onKeyEvent(const ::aidl::com::android::server::inputflinger::KeyEvent& in_event, bool* _aidl_return) override;
  ::ndk::ScopedAStatus onMotionEvent(const ::aidl::android::hardware::input::common::MotionEvent& in_event, bool* _aidl_return) override;
};
}  // namespace compound
}  // namespace xtr
}  // namespace com
}  // namespace aidl
