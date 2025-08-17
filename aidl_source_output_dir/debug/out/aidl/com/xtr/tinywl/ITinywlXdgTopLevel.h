/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/debug/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl5312348015633281126.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl/com/xtr/tinywl/ITinywlXdgTopLevel.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <aidl/com/xtr/tinywl/XdgTopLevel.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl::com::xtr::tinywl {
class XdgTopLevel;
}  // namespace aidl::com::xtr::tinywl
namespace aidl {
namespace com {
namespace xtr {
namespace tinywl {
class ITinywlXdgTopLevelDelegator;

class ITinywlXdgTopLevel : public ::ndk::ICInterface {
public:
  typedef ITinywlXdgTopLevelDelegator DefaultDelegator;
  static const char* descriptor;
  ITinywlXdgTopLevel();
  virtual ~ITinywlXdgTopLevel();

  static constexpr uint32_t TRANSACTION_addXdgTopLevel = FIRST_CALL_TRANSACTION + 0;
  static constexpr uint32_t TRANSACTION_removeXdgTopLevel = FIRST_CALL_TRANSACTION + 1;

  static std::shared_ptr<ITinywlXdgTopLevel> fromBinder(const ::ndk::SpAIBinder& binder);
  static binder_status_t writeToParcel(AParcel* parcel, const std::shared_ptr<ITinywlXdgTopLevel>& instance);
  static binder_status_t readFromParcel(const AParcel* parcel, std::shared_ptr<ITinywlXdgTopLevel>* instance);
  static bool setDefaultImpl(const std::shared_ptr<ITinywlXdgTopLevel>& impl);
  static const std::shared_ptr<ITinywlXdgTopLevel>& getDefaultImpl();
  virtual ::ndk::ScopedAStatus addXdgTopLevel(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) = 0;
  virtual ::ndk::ScopedAStatus removeXdgTopLevel(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) = 0;
private:
  static std::shared_ptr<ITinywlXdgTopLevel> default_impl;
};
class ITinywlXdgTopLevelDefault : public ITinywlXdgTopLevel {
public:
  ::ndk::ScopedAStatus addXdgTopLevel(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
  ::ndk::ScopedAStatus removeXdgTopLevel(const ::aidl::com::xtr::tinywl::XdgTopLevel& in_xdgToplevel) override;
  ::ndk::SpAIBinder asBinder() override;
  bool isRemote() override;
};
}  // namespace tinywl
}  // namespace xtr
}  // namespace com
}  // namespace aidl
