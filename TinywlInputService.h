#pragma once
#ifdef __cplusplus

#include <aidl/tinywl/BnTinywlInput.h>

extern "C" {
  #include "tinywl.h"
}

namespace tinywl {
  using namespace aidl::com::android::server::inputflinger;
  using namespace aidl::android::hardware::input::common;

  class TinywlInputService : public aidl::tinywl::BnTinywlInput {
    public:
      int32_t width;
      int32_t height;
      ::ndk::ScopedAStatus onKeyEvent(const KeyEvent& in_event, long in_nativePtr, bool* _aidl_return) override;
      ::ndk::ScopedAStatus onMotionEvent(const MotionEvent& in_event, long in_nativePtr, bool* _aidl_return) override;
      void setTinywlServer(struct tinywl_server* server);
    
    private:
      void sendPointerButtonEvent(const MotionEvent& in_event, struct tinywl_toplevel *toplevel);
      void sendPointerPosition(const MotionEvent& in_event, struct tinywl_toplevel *toplevel);
      void sendScrollEvent(const MotionEvent& in_event, struct tinywl_toplevel *toplevel);
      struct wlr_keyboard keyboard;
      struct wlr_pointer pointer;
      struct tinywl_server* server;
  };
}  // namespace tinywl

std::shared_ptr<tinywl::TinywlInputService> TinywlInputService_make();

#endif
