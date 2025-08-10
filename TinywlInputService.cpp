#include <aidl/tinywl/BnTinywlInput.h>
#include "TinywlInputService.h"

namespace tinywl {
  using namespace aidl::com::android::server::inputflinger;
  using namespace aidl::android::hardware::input::common;

  class TinywlInputService : public aidl::tinywl::BnTinywlInput {
    
  public:
    ::ndk::ScopedAStatus onKeyEvent(const KeyEvent& in_event, bool* _aidl_return) override {
      return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onMotionEvent(const MotionEvent& in_event, bool* _aidl_return) override {
      return ::ndk::ScopedAStatus::ok();
    }
  };  // class TinywlInputService

}  // namespace tinywl

AIBinder* TinywlInputService_asBinder() {
    auto service = ndk::SharedRefBase::make<tinywl::TinywlInputService>();
    auto binder = service->asBinder();
    service->fromBinder(binder);
    return binder.get();
}
