/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Using: /home/hp/Android/Sdk/build-tools/35.0.0/aidl --lang=ndk -o/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out/ndk -h/home/hp/Desktop/wlroots-android-bridge/tinywl/build/generated/aidl_source_output_dir/debug/out/ndk -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/debug/aidl -I/home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/cpp/aidl -d/tmp/aidl13310722393780283455.d /home/hp/Desktop/wlroots-android-bridge/tinywl/src/main/aidl/com/xtr/tinywl/XdgTopLevel.aidl
 */
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <android/binder_interface_utils.h>
#include <android/binder_parcelable_utils.h>
#include <android/binder_to_string.h>
#include <android/native_window_aidl.h>
#ifdef BINDER_STABILITY_SUPPORT
#include <android/binder_stability.h>
#endif  // BINDER_STABILITY_SUPPORT

namespace aidl {
namespace com {
namespace xtr {
namespace tinywl {
class XdgTopLevel {
public:
  typedef std::false_type fixed_size;
  static const char* descriptor;

  std::string appId;
  std::string title;
  ::aidl::android::view::Surface surface;
  std::string nativePtr;

  binder_status_t readFromParcel(const AParcel* parcel);
  binder_status_t writeToParcel(AParcel* parcel) const;

  inline bool operator==(const XdgTopLevel& _rhs) const {
    return std::tie(appId, title, surface, nativePtr) == std::tie(_rhs.appId, _rhs.title, _rhs.surface, _rhs.nativePtr);
  }
  inline bool operator<(const XdgTopLevel& _rhs) const {
    return std::tie(appId, title, surface, nativePtr) < std::tie(_rhs.appId, _rhs.title, _rhs.surface, _rhs.nativePtr);
  }
  inline bool operator!=(const XdgTopLevel& _rhs) const {
    return !(*this == _rhs);
  }
  inline bool operator>(const XdgTopLevel& _rhs) const {
    return _rhs < *this;
  }
  inline bool operator>=(const XdgTopLevel& _rhs) const {
    return !(*this < _rhs);
  }
  inline bool operator<=(const XdgTopLevel& _rhs) const {
    return !(_rhs < *this);
  }

  static const ::ndk::parcelable_stability_t _aidl_stability = ::ndk::STABILITY_LOCAL;
  inline std::string toString() const {
    std::ostringstream _aidl_os;
    _aidl_os << "XdgTopLevel{";
    _aidl_os << "appId: " << ::android::internal::ToString(appId);
    _aidl_os << ", title: " << ::android::internal::ToString(title);
    _aidl_os << ", surface: " << ::android::internal::ToString(surface);
    _aidl_os << ", nativePtr: " << ::android::internal::ToString(nativePtr);
    _aidl_os << "}";
    return _aidl_os.str();
  }
};
}  // namespace tinywl
}  // namespace xtr
}  // namespace com
}  // namespace aidl
