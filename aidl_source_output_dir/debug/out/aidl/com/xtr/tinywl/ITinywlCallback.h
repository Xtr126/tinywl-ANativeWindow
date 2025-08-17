/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/debug/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl14536866063595600578.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl/com/xtr/tinywl/ITinywlCallback.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <aidl/com/xtr/tinywl/ITinywlXdgTopLevel.h>
#include <aidl/com/xtr/tinywl/XdgTopLevel.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::com::xtr::tinywl {
class ITinywlXdgTopLevel;
class XdgTopLevel;
}  // namespace aidl::com::xtr::tinywl
namespace aidl {
namespace com {
namespace xtr {
namespace tinywl {
class ITinywlCallbackDelegator;

class ITinywlCallback : public ::ndk::ICInterface {
public:
  typedef ITinywlCallbackDelegator DefaultDelegator;
  static const char* descriptor;
  ITinywlCallback();
  virtual ~ITinywlCallback();

  static constexpr uint32_t TRANSACTION_startTinywl = FIRST_CALL_TRANSACTION + 0;
  static constexpr uint32_t TRANSACTION_onSurfaceCreated = FIRST_CALL_TRANSACTION + 1;
  static constexpr uint32_t TRANSACTION_onSurfaceChanged = FIRST_CALL_TRANSACTION + 2;
  static constexpr uint32_t TRANSACTION_onSurfaceDestroyed = FIRST_CALL_TRANSACTION + 3;

  static std::shared_ptr<ITinywlCallback> fromBinder(const ::ndk::SpAIBinder& binder);
  static binder_status_t writeToParcel(AParcel* parcel, const std::shared_ptr<ITinywlCallback>& instance);
  static binder_status_t readFromParcel(const AParcel* parcel, std::shared_ptr<ITinywlCallback>* instance);
  static bool setDefaultImpl(const std::shared_ptr<ITinywlCallback>& impl);
  static const std::shared_ptr<ITinywlCallback>& getDefaultImpl();
  virtual ::ndk::ScopedAStatus startTinywl(const std::shared_ptr<::aidl::com::xtr::tinywl::ITinywlXdgTopLevel>& in_xdgTopLevelCallback) = 0;
  virtual ::ndk::ScopedAStatus onSurfaceCreated(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) = 0;
  virtual ::ndk::ScopedAStatus onSurfaceChanged(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) = 0;
  virtual ::ndk::ScopedAStatus onSurfaceDestroyed(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) = 0;
private:
  static std::shared_ptr<ITinywlCallback> default_impl;
};
class ITinywlCallbackDefault : public ITinywlCallback {
public:
  ::ndk::ScopedAStatus startTinywl(const std::shared_ptr<::aidl::com::xtr::tinywl::ITinywlXdgTopLevel>& in_xdgTopLevelCallback) override;
  ::ndk::ScopedAStatus onSurfaceCreated(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
  ::ndk::ScopedAStatus onSurfaceChanged(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
  ::ndk::ScopedAStatus onSurfaceDestroyed(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
  ::ndk::SpAIBinder asBinder() override;
  bool isRemote() override;
};
}  // namespace tinywl
}  // namespace xtr
}  // namespace com
}  // namespace aidl
